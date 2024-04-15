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

  const heritageClausesData = handleClassEntityHeritageClauses(rootName, classEntity);
  const isExtend = heritageClausesData.isExtend;
  classBody = addCustomeClass(heritageClausesData, sourceFile) + classBody;
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
function handleClassEntityHeritageClauses(rootName: string, classEntity: ClassEntity): { isExtend: boolean, classBody: string } {
  let isExtend = false;
  let classBody = '';
  if (classEntity.heritageClauses.length > 0) {
    classEntity.heritageClauses.forEach(value => {
      if (value.clauseToken === 'extends') {
        isExtend = true;
        classBody += `${value.clauseToken} `;
        value.types.forEach((val, index) => {
          const extendClassName = val.split('<')[0];
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
          } else if (val === 'uri.URI') {
            classBody += 'mockUri().URI';
          } else if (val === 'photoAccessHelper.BaseSelectOptions') {
            classBody += 'mockPhotoAccessHelper().BaseSelectOptions';
          } else {
            classBody += `${extendClassName}`;
          }
        });
      }
    });
  }
  return {
    isExtend,
    classBody
  };
}

/**
 * add custome class
 * @param heritageClausesData
 * @param sourceFile
 * @returns
 */
function addCustomeClass(heritageClausesData: {isExtend: boolean, classBody:string}, sourceFile: SourceFile) :string {
  if (!heritageClausesData.isExtend) {
    return '';
  }
  if (!path.resolve(sourceFile.fileName).includes(path.join('@internal', 'component', 'ets'))) {
    return '';
  }
  let mockClassBody = '';
  if (heritageClausesData.classBody.startsWith('extends ')) {
    const classArr = heritageClausesData.classBody.split('extends');
    const className = classArr[classArr.length - 1].trim();
    if (className !== 'extends') {
      const removeNoteRegx = /\/\*[\s\S]*?\*\//g;
      const fileContent = sourceFile.getText().replace(removeNoteRegx, '');
      const regex = new RegExp(`\\sclass\\s*${className}\\s*(<|{|extends)`);
      const results = fileContent.match(regex);
      if (!results) {
        mockClassBody = `class ${className} {};\n`;
      }
    }
  }
  return mockClassBody;
}
