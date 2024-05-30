/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import path from 'path';
import type { SourceFile } from 'typescript';
import { SyntaxKind } from 'typescript';
import { firstCharacterToUppercase } from '../common/commonUtils';
import type { ClassEntity } from '../declaration-node/classDeclaration';
import { generateCommonMethod } from './generateCommonMethod';
import { getWarnConsole } from './generateCommonUtil';
import { generatePropertyDeclaration } from './generatePropertyDeclaration';
import { generateStaticFunction } from './generateStaticFunction';
import { ImportElementEntity } from '../declaration-node/importAndExportDeclaration';
import { HeritageClauseEntity } from '../declaration-node/heritageClauseDeclaration';

interface AssemblyClassParams {
  isSystem: boolean,
  classEntity: ClassEntity,
  classBody: string,
  sourceFile: SourceFile,
  mockApi: string,
  isInnerMockFunction: boolean,
  filename: string,
  isExtend: boolean,
  className: string,
  extraImport?: string[],
  importDeclarations?: ImportElementEntity[]
}

/**
 * generate class
 * @param rootName
 * @param classEntity
 * @param isSystem
 * @param globalName
 * @param filename
 * @param sourceFile
 * @param isInnerMockFunction
 * @returns
 */
export function generateClassDeclaration(
  rootName: string,
  classEntity: ClassEntity,
  isSystem: boolean,
  globalName: string,
  filename: string,
  sourceFile: SourceFile,
  isInnerMockFunction: boolean,
  mockApi: string,
  extraImport?: string[],
  importDeclarations?: ImportElementEntity[]
): string {
  if (isSystem) {
    return '';
  }

  const className = firstCharacterToUppercase(classEntity.className);
  let classBody = '';
  if ((classEntity.exportModifiers.includes(SyntaxKind.ExportKeyword) ||
    classEntity.exportModifiers.includes(SyntaxKind.DeclareKeyword)) &&
    !isInnerMockFunction) {
    classBody += `export const ${className} = class ${className} `;
  } else {
    classBody += `const ${className} = class ${className} `;
  }

  const heritageClausesData = handleClassEntityHeritageClauses(rootName, classEntity, mockApi, sourceFile);
  const isExtend = heritageClausesData.isExtend;
  classBody = addCustomeClass(heritageClausesData, sourceFile, importDeclarations) + classBody;
  classBody += heritageClausesData.classBody;
  classBody = assemblyClassBody({
    isSystem,
    classEntity,
    classBody,
    className,
    isExtend,
    sourceFile,
    mockApi,
    isInnerMockFunction,
    filename,
    extraImport,
    importDeclarations
  });
  return classBody;
}

/**
 * generate some class
 * @param porps
 * @returns
 */
function assemblyClassBody(porps: AssemblyClassParams): string {
  if (!porps.isSystem) {
    porps.classBody += '{';
    if (porps.classEntity.classConstructor.length > 1) {
      porps.classBody += 'constructor(...arg) { ';
    } else {
      porps.classBody += 'constructor() { ';
    }
    if (porps.isExtend) {
      porps.classBody += 'super();\n';
    }
    const warnCon = getWarnConsole(porps.className, 'constructor');
    porps.classBody += porps.sourceFile.fileName.endsWith('PermissionRequestResult.d.ts') ? '' : warnCon;
  }
  if (porps.classEntity.classProperty.length > 0) {
    porps.classEntity.classProperty.forEach(value => {
      porps.classBody += generatePropertyDeclaration(porps.className, value,
        porps.sourceFile, porps.extraImport, porps.importDeclarations) + '\n';
    });
  }

  if (porps.classEntity.classMethod.size > 0) {
    porps.classEntity.classMethod.forEach(value => {
      porps.classBody += generateCommonMethod(porps.className, value, porps.sourceFile, porps.mockApi);
    });
  }

  porps.classBody += '}\n};';
  porps.classBody = assemblyGlobal(porps);

  if (!porps.filename.startsWith('system_')) {
    if (porps.classEntity.staticMethods.length > 0) {
      let staticMethodBody = '';
      porps.classEntity.staticMethods.forEach(value => {
        staticMethodBody += generateStaticFunction(value, false, porps.sourceFile, porps.mockApi) + '\n';
      });
      porps.classBody += staticMethodBody;
    }
  }
  if (porps.classEntity.exportModifiers.includes(SyntaxKind.DefaultKeyword)) {
    porps.classBody += `\nexport default ${porps.className};`;
  }
  return porps.classBody;
}

/**
 * generate some class
 * @param porps
 * @returns
 */
function assemblyGlobal(porps: AssemblyClassParams): string {
  if (
    (porps.classEntity.exportModifiers.includes(SyntaxKind.ExportKeyword) ||
      porps.classEntity.exportModifiers.includes(SyntaxKind.DeclareKeyword)) &&
    !porps.isInnerMockFunction
  ) {
    porps.classBody += `
      if (!global.${porps.className}) {
        global.${porps.className} = ${porps.className};\n
      }
    `;
  }
  return porps.classBody;
}

/**
 * generate class
 * @param rootName
 * @param classEntity
 * @returns
 */
function handleClassEntityHeritageClauses(
  rootName: string,
  classEntity: ClassEntity,
  mockApi: string,
  sourceFile: SourceFile
): { isExtend: boolean, classBody: string } {
  let isExtend = false;
  let classBody = '';
  if (classEntity.heritageClauses.length > 0) {
    classEntity.heritageClauses.forEach(value => {
      if (value.clauseToken === 'extends') {
        isExtend = true;
        classBody += `${value.clauseToken} `;
        classBody = generateClassEntityHeritageClauses(classEntity, value, classBody, rootName, mockApi, sourceFile);
      }
    });
  }
  return {
    isExtend,
    classBody
  };
}

/**
 * generate classEntity heritageClauses
 * @param classEntity
 * @param value
 * @param classBody
 * @param rootName
 * @param mockApi
 * @param sourceFile
 * @returns
 */
function generateClassEntityHeritageClauses(
  classEntity: ClassEntity,
  value: HeritageClauseEntity,
  classBody: string,
  rootName: string,
  mockApi: string,
  sourceFile: SourceFile
): string {
  value.types.forEach((val, index) => {
    const extendClassName = val.trim().split('<')[0];
    const moduleName = firstCharacterToUppercase(rootName);
    if (val.startsWith('Array<')) {
      val = 'Array';
    } else {
      if (classEntity.exportModifiers.includes(SyntaxKind.ExportKeyword) && rootName !== '') {
        val = `mock${moduleName}().${val}`;
      }
    }
    if (index !== value.types.length - 1) {
      classBody += `${extendClassName},`;
    } else if (val.includes('.')) {
      const name = val.split('.')[0];
      if (mockApi.includes(`import { mock${firstCharacterToUppercase(name)} }`) &&
            path.basename(sourceFile.fileName).startsWith('@ohos.')) {
        classBody += val.replace(name, `mock${firstCharacterToUppercase(name)}()`);
      } else {
        classBody += `${extendClassName}`;
      }
    } else {
      classBody += `${extendClassName}`;
    }
  });
  return classBody;
}

/**
 * add custome class
 * @param heritageClausesData
 * @param sourceFile
 * @returns
 */
function addCustomeClass(
  heritageClausesData: {isExtend: boolean, classBody:string},
  sourceFile: SourceFile,
  importDeclarations?: ImportElementEntity[]
): string {
  if (!heritageClausesData.isExtend) {
    return '';
  }
  if (
    !path.resolve(sourceFile.fileName).includes(path.join('@internal', 'component', 'ets')) &&
    path.basename(sourceFile.fileName).startsWith('@ohos.')
  ) {
    return '';
  }
  let mockClassBody = '';
  if (!heritageClausesData.classBody.startsWith('extends ')) {
    return mockClassBody;
  }
  const classArr = heritageClausesData.classBody.split('extends');
  const className = classArr[classArr.length - 1].trim();
  if (className === 'extends') {
    return mockClassBody;
  }
  const removeNoteRegx = /\/\*[\s\S]*?\*\//g;
  const fileContent = sourceFile.getText().replace(removeNoteRegx, '');
  let hasImportType = false;
  if (importDeclarations) {
    importDeclarations.forEach(element => {
      if (element.importElements.includes(className)) {
        hasImportType = true;
      }
    });
  }
  const regex = new RegExp(`\\sclass\\s*${className}\\s*(<|{|extends|implements)`);
  const results = fileContent.match(regex);
  if (!results && !hasImportType) {
    mockClassBody = `class ${className} {};\n`;
  }
  return mockClassBody;
}
