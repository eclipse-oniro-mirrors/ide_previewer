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

import fs from 'fs';
import path from 'path';
import { ScriptTarget, SyntaxKind, createSourceFile } from 'typescript';
import type { SourceFile } from 'typescript';
import { collectAllLegalImports, dtsFileList, firstCharacterToUppercase, getAllFileNameList, getApiInputPath } from '../common/commonUtils';
import type { ImportElementEntity } from '../declaration-node/importAndExportDeclaration';
import { getDefaultExportClassDeclaration } from '../declaration-node/sourceFileElementsAssemply';
import type { SourceFileEntity } from '../declaration-node/sourceFileElementsAssemply';
import { generateClassDeclaration } from './generateClassDeclaration';
import { generateEnumDeclaration } from './generateEnumDeclaration';
import { addToIndexArray } from './generateIndex';
import { generateInterfaceDeclaration } from './generateInterfaceDeclaration';
import { generateModuleDeclaration } from './generateModuleDeclaration';
import { generateStaticFunction } from './generateStaticFunction';
import { addToSystemIndexArray } from './generateSystemIndex';
import { generateTypeAliasDeclaration } from './generateTypeAlias';
import { generateExportFunction } from './generateExportFunction';

/**
 * generate mock file string
 * @param rootName
 * @param sourceFileEntity
 * @param sourceFile
 * @param fileName
 * @returns
 */
export function generateSourceFileElements(rootName: string, sourceFileEntity: SourceFileEntity, sourceFile: SourceFile, fileName: string): string {
  
  let mockApi = '';
  const mockFunctionElements: Array<MockFunctionElementEntity> = [];
  const dependsSourceFileList = collectReferenceFiles(sourceFile);
  const heritageClausesArray = getCurrentApiHeritageArray(sourceFileEntity, sourceFile);
  const extraImport = [];

  if (sourceFileEntity.importDeclarations.length > 0) {
    sourceFileEntity.importDeclarations.forEach(value => {
      mockApi += generateImportDeclaration(value, fileName, heritageClausesArray, sourceFile.fileName, dependsSourceFileList);
    });
  }

  if (sourceFileEntity.moduleDeclarations.length > 0) {
    sourceFileEntity.moduleDeclarations.forEach(value => {
      mockApi += generateModuleDeclaration('', value, sourceFile, fileName, mockApi, extraImport) + '\n';
    });
  }

  if (sourceFileEntity.classDeclarations.length > 0) {
    sourceFileEntity.classDeclarations.forEach(value => {
      if (!fileName.startsWith('system_') && !value.exportModifiers.includes(SyntaxKind.DefaultKeyword)) {
        mockApi += generateClassDeclaration('', value, false, '', fileName, sourceFile, false, mockApi) + '\n';
        mockFunctionElements.push({ elementName: value.className, type: 'class' });
      }
    });
  }

  if (sourceFileEntity.interfaceDeclarations.length > 0) {
    sourceFileEntity.interfaceDeclarations.forEach(value => {
      mockApi += generateInterfaceDeclaration('', value, sourceFile, true, mockApi, sourceFileEntity.interfaceDeclarations,
        sourceFileEntity.importDeclarations, extraImport) + '\n';
      mockFunctionElements.push({ elementName: value.interfaceName, type: 'interface' });
    });
  }

  if (sourceFileEntity.enumDeclarations.length > 0) {
    sourceFileEntity.enumDeclarations.forEach(value => {
      mockApi += generateEnumDeclaration('', value) + '\n';
      mockFunctionElements.push({ elementName: value.enumName, type: 'enum' });
    });
  }

  if (sourceFileEntity.typeAliasDeclarations.length > 0) {
    sourceFileEntity.typeAliasDeclarations.forEach(value => {
      mockApi += generateTypeAliasDeclaration(value, false, sourceFile, extraImport) + '\n';
      mockFunctionElements.push({ elementName: value.typeAliasName, type: 'typeAlias' });
    });
  }

  if (sourceFileEntity.functionDeclarations.length > 0) {
    sourceFileEntity.functionDeclarations.forEach(value => {
      mockApi += generateExportFunction(value, sourceFile, mockApi) + '\n';
    });
  }

  if (sourceFileEntity.moduleDeclarations.length === 0 && (fileName.startsWith('ohos_') || fileName.startsWith('system_') || fileName.startsWith('webgl'))) {
    const mockNameArr = fileName.split('_');
    const mockName = mockNameArr[mockNameArr.length - 1];
    const defaultExportClass = getDefaultExportClassDeclaration(sourceFile);
    if (defaultExportClass.length > 0) {
      defaultExportClass.forEach(value => {
        mockApi += generateClassDeclaration(rootName, value, false, mockName, '', sourceFile, false, mockApi) + '\n';
        mockFunctionElements.push({ elementName: value.className, type: 'class' });
      });
    }
    mockApi += `export function mock${firstCharacterToUppercase(mockName)}() {\n`;
    if (fileName.startsWith('system_')) {
      addToSystemIndexArray({
        filename: fileName,
        mockFunctionName: `mock${firstCharacterToUppercase(mockName)}`
      });
      mockApi += `global.systemplugin.${mockName} = {`;
      const defaultClass = getDefaultExportClassDeclaration(sourceFile);
      let staticMethodBody = '';
      if (defaultClass.length > 0) {
        defaultClass.forEach(value => {
          value.staticMethods.forEach(val => {
            staticMethodBody += generateStaticFunction(val, true, sourceFile, mockApi);
          });
        });
      }
      mockApi += staticMethodBody;
      mockApi += '}';
    } else {
      if (!fileName.startsWith('webgl')) {
        addToIndexArray({ fileName: fileName, mockFunctionName: `mock${firstCharacterToUppercase(mockName)}` });
      }
    }
    mockApi += `\nconst mockModule${firstCharacterToUppercase(mockName)} = {`;
    mockFunctionElements.forEach(val => {
      mockApi += `${val.elementName}: ${val.elementName},`;
    });
    mockApi += '}\n';
    mockApi += `return mockModule${firstCharacterToUppercase(mockName)}.${firstCharacterToUppercase(mockName)}\n`;
    mockApi += '}';
  } else {
    const defaultExportClass = getDefaultExportClassDeclaration(sourceFile);
    if (defaultExportClass.length > 0) {
      const mockNameArr = fileName.split('_');
      const mockName = mockNameArr[mockNameArr.length - 1];
      defaultExportClass.forEach(value => {
        mockApi += generateClassDeclaration(rootName, value, false, mockName, '', sourceFile, false, mockApi) + '\n';
      });
    }
  }
  if (sourceFileEntity.exportDeclarations.length > 0) {
    sourceFileEntity.exportDeclarations.forEach(value => {
      if (value.includes('export type {')) {
        return;
      }
      if (!value.includes('export {')) {
        mockApi += `${value}\n`;
      }
    });
  }
  mockApi = extraImport.join('') + mockApi;
  return mockApi;
}

/**
 * generate import definition
 * @param importEntity
 * @param sourceFileName
 * @param heritageClausesArray
 * @param currentFilePath
 * @param dependsSourceFileList
 * @returns
 */
export function generateImportDeclaration(
  importEntity: ImportElementEntity,
  sourceFileName: string,
  heritageClausesArray: string[],
  currentFilePath: string,
  dependsSourceFileList: SourceFile[]): string {
  const importDeclaration = referenctImport2ModuleImport(importEntity, currentFilePath, dependsSourceFileList);
  if (importDeclaration) {
    return importDeclaration;
  }
  
  const importPathSplit = importEntity.importPath.split('/');

  let importPath = importPathSplit.slice(0, -1).join('/') + '/';
  importPath += getImportPathName(importPathSplit);

  let importElements = generateImportElements(importEntity, heritageClausesArray);
  if (importElements === '{ mockWantAgent }' && importPath.includes('ohos_app_ability_wantAgent')) {
    importElements = '{ mockWantAgent as mockAbilityWantAgent }';
  }
  const testPath = importPath.replace(/"/g, '').replace(/'/g, '').split('/');
  if (!getAllFileNameList().has(testPath[testPath.length - 1]) && testPath[testPath.length - 1] !== 'ohos_application_want') {
    return '';
  }

  let tmpImportPath = importPath.replace(/'/g, '').replace(/"/g, '');
  if (!tmpImportPath.startsWith('./') && !tmpImportPath.startsWith('../')) {
    importPath = `'./${tmpImportPath}'`;
  }
  if (sourceFileName === 'tagSession' && tmpImportPath === './basic' || sourceFileName === 'notificationContent' &&
  tmpImportPath === './ohos_multimedia_image') {
    importPath = `'.${importPath.replace(/'/g, '')}'`;
  }

  if (sourceFileName === 'AbilityContext' && tmpImportPath === '../ohos_application_Ability' ||
    sourceFileName === 'Context' && tmpImportPath === './ApplicationContext') {
    return '';
  }
  collectAllLegalImports(importElements);
  return `import ${importElements} from ${importPath}\n`;
}

/**
 * adapter default export
 * @param importName
 * @returns
 */
function checIsDefaultExportClass(importName: string): boolean {
  const defaultExportClass = ['Context', 'BaseContext', 'ExtensionContext', 'ApplicationContext',
    'ExtensionAbility', 'Ability', 'UIExtensionAbility', 'UIExtensionContext'];
  return defaultExportClass.includes(importName);
}

/**
 * get heritage elements
 * @param sourceFileEntity
 * @param sourceFile
 * @returns
 */
function getCurrentApiHeritageArray(sourceFileEntity: SourceFileEntity, sourceFile: SourceFile): string[] {
  const heritageClausesArray = [];
  const defaultClassArray = getDefaultExportClassDeclaration(sourceFile);
  sourceFileEntity.classDeclarations.forEach(value => {
    value.heritageClauses.forEach(val => {
      val.types.forEach(v => {
        heritageClausesArray.push(v);
      });
    });
  });
  defaultClassArray.forEach(value => {
    value.heritageClauses.forEach(val => {
      val.types.forEach(v => {
        heritageClausesArray.push(v);
      });
    });
  });
  return heritageClausesArray;
}

function collectReferenceFiles(sourceFile: SourceFile): SourceFile[] {
  const referenceElementTemplate = /\/\/\/\s*<reference\s+path="[^'"\[\]]+/g;
  const referenceFiles: SourceFile[] = [];
  const text = sourceFile.text;
  const referenceElement = text.match(referenceElementTemplate);

  referenceElement && referenceElement.forEach(element => {
    const referenceRelatePath = element.split(/path=["']/g)[1];
    const realReferenceFilePath = contentRelatePath2RealRelatePath(sourceFile.fileName, referenceRelatePath);
    if (!realReferenceFilePath) {
      return;
    }

    if (!fs.existsSync(realReferenceFilePath)) {
      console.error(`Can not resolve file: ${realReferenceFilePath}`);
      return;
    }
    const code = fs.readFileSync(realReferenceFilePath);
    referenceFiles.push(createSourceFile(realReferenceFilePath, code.toString(), ScriptTarget.Latest));
    !dtsFileList.includes(realReferenceFilePath) && dtsFileList.push(realReferenceFilePath);
  });
  return referenceFiles;
}

function contentRelatePath2RealRelatePath(currentFilePath: string, contentReferenceRelatePath: string): string {
  const conmponentSourceFileTemplate = /component\/[^'"\/]+\.d\.ts/;
  const currentFolderSourceFileTemplate = /\.\/[^\/]+\.d\.ts/;
  const baseFileNameTemplate = /[^\/]+\.d\.ts/;

  let realReferenceFilePath: string;
  if (conmponentSourceFileTemplate.test(contentReferenceRelatePath)) {
    const newRelateReferencePath = contentReferenceRelatePath.match(conmponentSourceFileTemplate)[0];
    const referenceFileName = path.basename(newRelateReferencePath);
    realReferenceFilePath = path.join(getApiInputPath(), '@internal', 'component', 'ets', referenceFileName);
  } else if (currentFolderSourceFileTemplate.test(contentReferenceRelatePath)) {
    const referenceFileName = path.basename(contentReferenceRelatePath);
    realReferenceFilePath = currentFilePath.replace(baseFileNameTemplate, referenceFileName).replace(/\//g, path.sep);
  } else {
    console.error(`Can not find reference ${contentReferenceRelatePath} from ${currentFilePath}`);
    return '';
  }
  return realReferenceFilePath;
}

export function referenctImport2ModuleImport(importEntity: ImportElementEntity, currentFilePath: string,
  dependsSourceFileList: SourceFile[]): string {
  if (dependsSourceFileList.length && !importEntity.importPath.includes('.')) {
    for (let i = 0; i < dependsSourceFileList.length; i++) {
      if (dependsSourceFileList[i].text.includes(`declare module ${importEntity.importPath.replace(/'/g, '"')}`)) {
        let relatePath = path.relative(path.dirname(currentFilePath), dependsSourceFileList[i].fileName)
          .replace(/\\/g, '/')
          .replace(/.d.ts/g, '')
          .replace(/.d.es/g, '');
        relatePath = (relatePath.startsWith('@internal/component') ? './' : '') + relatePath;
        return `import ${importEntity.importElements} from "${relatePath}"\n`;
      }
    }
  }
  return '';
}

function getImportPathName(importPathSplit: string[]): string {
  let importPathName: string;
  let fileName = importPathSplit[importPathSplit.length - 1];
  if (fileName.endsWith('.d.ts') || fileName.endsWith('.d.ets')) {
    fileName = fileName.split(/\.d\.e?ts/)[0];
  }
  if (fileName.includes('@')) {
    importPathName = fileName.replace('@', '').replace(/\./g, '_');
  } else {
    importPathName = fileName.replace(/\./g, '_');
  }
  return importPathName;
}

function generateImportElements(importEntity: ImportElementEntity, heritageClausesArray: string[]): string {
  let importElements = importEntity.importElements;
  if (!importElements.includes('{') && !importElements.includes('* as') && !heritageClausesArray.includes(importElements) && importEntity.importPath.includes('@ohos')) {
    const tmpArr = importEntity.importPath.split('.');
    importElements = `{ mock${firstCharacterToUppercase(tmpArr[tmpArr.length - 1].replace('"', '').replace('\'', ''))} }`;
  } else {
    // adapt no rules .d.ts
    if (importElements.trimRight().trimEnd() === 'AccessibilityExtensionContext, { AccessibilityElement }') {
      importElements = '{ AccessibilityExtensionContext, AccessibilityElement }';
    } else if (importElements.trimRight().trimEnd() === '{ image }') {
      importElements = '{ mockImage as image }';
    }
  }
  return importElements;
}


interface MockFunctionElementEntity {
  elementName: string,
  type: string
}
