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
import {
  ScriptTarget,
  SyntaxKind,
  createSourceFile
} from 'typescript';
import type { SourceFile } from 'typescript';
import {
  collectAllLegalImports,
  dtsFileList,
  firstCharacterToUppercase,
  getAllFileNameList,
  getApiInputPath
} from '../common/commonUtils';
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
import { generateCommonFunction } from './generateCommonFunction';
import {
  needToAddBrace,
  hasExportDefaultKeyword,
  MockFunctionElementEntity,
  ReturnDataParams,
  needAddExtraClass
} from './generateCommonUtil';

/**
 * generate mock file string
 * @param rootName absolute path to the mock file to be generated
 * @param sourceFileEntity all node information in the file
 * @param sourceFile file Text Information
 * @param fileName file name
 * @returns file mock text
 */
export function generateSourceFileElements(
  rootName: string,
  sourceFileEntity: SourceFileEntity,
  sourceFile: SourceFile,
  fileName: string
): string {
  let mockApi = '';
  let mockFunctionElements: Array<MockFunctionElementEntity> = [];
  const dependsSourceFileList = collectReferenceFiles(sourceFile);
  const heritageClausesArray = getCurrentApiHeritageArray(sourceFileEntity, sourceFile);
  const extraImport = [];

  mockApi += importDeclarationsGenerate(sourceFileEntity, sourceFile, fileName, heritageClausesArray, dependsSourceFileList);
  const enumDeclarationsData = enumDeclarationsGenerate(sourceFileEntity, mockFunctionElements);
  mockApi += enumDeclarationsData.mockData;
  mockFunctionElements = enumDeclarationsData.mockFunctionElements;

  const typeAliasDeclarationsData = typeAliasDeclarationsGenerate(sourceFileEntity, mockFunctionElements, sourceFile, extraImport, mockApi);
  mockApi += typeAliasDeclarationsData.mockData;
  mockFunctionElements = typeAliasDeclarationsData.mockFunctionElements;

  const interfaceDeclarationsData = interfaceDeclarationsGenerate(sourceFileEntity, mockFunctionElements, sourceFile, extraImport, mockApi);
  mockApi += interfaceDeclarationsData.mockData;
  mockFunctionElements = interfaceDeclarationsData.mockFunctionElements;

  const classDeclarationsData = classDeclarationsGenerate(sourceFileEntity, mockFunctionElements, sourceFile, mockApi, fileName);
  mockApi += classDeclarationsData.mockData;
  mockFunctionElements = classDeclarationsData.mockFunctionElements;

  mockApi += moduleDeclarationsGenerate(sourceFileEntity, sourceFile, mockApi, fileName, extraImport);

  mockApi += functionDeclarationsGenerate(sourceFileEntity, sourceFile, mockApi);

  mockApi += otherDeclarationsGenerate(rootName, sourceFileEntity, mockFunctionElements, sourceFile, mockApi, fileName).mockData;

  mockApi += handleExportDeclarations(sourceFileEntity);

  mockApi = extraImport.join('') + mockApi;

  mockApi = addExportDefaultExpression(mockApi);

  return mockApi;
}

/**
 * get import declarations generate
 * @param sourceFileEntity all node information in the file
 * @param sourceFile file Text Information
 * @param fileName file name
 * @param heritageClausesArray  heritage elements array data
 * @param dependsSourceFileList reference Files data
 * @returns string
 */
function importDeclarationsGenerate(
  sourceFileEntity: SourceFileEntity,
  sourceFile: SourceFile,
  fileName: string,
  heritageClausesArray: string[],
  dependsSourceFileList: SourceFile[]
): string {
  let mockData = '';
  if (needAddExtraClass.includes(`${fileName}.d.ts`)) {
    mockData += 'import { CommonMethod } from \'./common\';\n';
  }
  if (sourceFileEntity.importDeclarations.length > 0) {
    sourceFileEntity.importDeclarations.forEach(value => {
      if (
        sourceFile.fileName.endsWith('@ohos.arkui.UIContext.d.ts') &&
        ['\'DatePickerDialogParam\'', '\'TimePickerDialogParam\'', '\'textPickerDialogParam\''].includes(value.importPath)
      ) {
        mockData += '';
      } else {
        mockData += generateImportDeclaration(value, fileName, heritageClausesArray, sourceFile.fileName, dependsSourceFileList);
      }
    });
  }
  if (fileName === 'ohos_arkui_observer') {
    mockData += 'const NavigationOperation = { PUSH: 1, POP: 2, REPLACE: 3 }\n';
  }
  if (fileName === 'ImageModifier') {
    mockData += 'class ImageAttribute {}\n';
  }
  return mockData;
}

/**
 * get enum declarations generate
 * @param sourceFileEntity all node information in the file
 * @param mockFunctionElements all function element entity
 * @returns ReturnDataParams
 */
function enumDeclarationsGenerate(
  sourceFileEntity: SourceFileEntity,
  mockFunctionElements: Array<MockFunctionElementEntity>
): ReturnDataParams {
  const data: ReturnDataParams = {
    mockData: '',
    mockFunctionElements: mockFunctionElements
  };
  if (sourceFileEntity.enumDeclarations.length > 0) {
    sourceFileEntity.enumDeclarations.forEach(value => {
      data.mockData += generateEnumDeclaration('', value) + '\n';
      data.mockFunctionElements.push({ elementName: value.enumName, type: 'enum' });
    });
  }
  return data;
}

/**
 * get typeAlias declarations generate
 * @param sourceFileEntity all node information in the file
 * @param mockFunctionElements all function element entity
 * @param sourceFile file Text Information
 * @param extraImport extra import data
 * @param mockApi file mock text
 * @returns ReturnDataParams
 */
function typeAliasDeclarationsGenerate(
  sourceFileEntity: SourceFileEntity,
  mockFunctionElements: Array<MockFunctionElementEntity>,
  sourceFile: SourceFile,
  extraImport: string[],
  mockApi: string
): ReturnDataParams {
  const data: ReturnDataParams = {
    mockData: '',
    mockFunctionElements: mockFunctionElements
  };
  if (sourceFileEntity.typeAliasDeclarations.length > 0) {
    sourceFileEntity.typeAliasDeclarations.forEach(value => {
      data.mockData += generateTypeAliasDeclaration(value, false, sourceFile, extraImport, mockApi) + '\n';
      data.mockFunctionElements.push({ elementName: value.typeAliasName, type: 'typeAlias' });
    });
  }
  return data;
}

/**
 * get interface declarations generate
 * @param sourceFileEntity all node information in the file
 * @param mockFunctionElements all function element entity
 * @param sourceFile file Text Information
 * @param extraImport Additional exported data
 * @param mockApi  file mock into text
 * @returns ReturnDataParams
 */
function interfaceDeclarationsGenerate(
  entity: SourceFileEntity,
  mockFunctionElements: Array<MockFunctionElementEntity>,
  sourceFile: SourceFile,
  extraImport: string[],
  mockApi: string
): ReturnDataParams {
  const data: ReturnDataParams = {
    mockData: '',
    mockFunctionElements: mockFunctionElements
  };
  if (entity.interfaceDeclarations.length > 0) {
    entity.interfaceDeclarations.forEach(value => {
      data.mockData += generateInterfaceDeclaration(
        value, sourceFile, true, mockApi, entity.interfaceDeclarations,
        entity.importDeclarations, extraImport
      ) + '\n';
      data.mockFunctionElements.push({ elementName: value.interfaceName, type: 'interface' });
    });
  }
  return data;
}

/**
 * get class declarations generate
 * @param sourceFileEntity all node information in the file
 * @param mockFunctionElements all function element entity
 * @param sourceFile file Text Information
 * @param mockApi file mock text
 * @param fileName file name
 * @returns ReturnDataParams
 */
function classDeclarationsGenerate(
  sourceFileEntity: SourceFileEntity,
  mockFunctionElements: Array<MockFunctionElementEntity>,
  sourceFile: SourceFile,
  mockApi: string,
  fileName: string
): ReturnDataParams {
  const data: ReturnDataParams = {
    mockData: '',
    mockFunctionElements: mockFunctionElements
  };
  if (sourceFileEntity.classDeclarations.length > 0) {
    sourceFileEntity.classDeclarations.forEach(value => {
      if (!fileName.startsWith('system_') && !value.exportModifiers.includes(SyntaxKind.DefaultKeyword)) {
        data.mockData += generateClassDeclaration('', value, false, '', fileName, sourceFile, false, mockApi) + '\n';
        data.mockFunctionElements.push({ elementName: value.className, type: 'class' });
      }
    });
  }
  return data;
}

/**
 * get module declarations generate
 * @param sourceFileEntity all node information in the file
 * @param sourceFile file Text Information
 * @param mockApi file mock text
 * @param fileName  file name
 * @param extraImport  extra import data
 * @returns string
 */
function moduleDeclarationsGenerate(
  sourceFileEntity: SourceFileEntity,
  sourceFile: SourceFile,
  mockApi: string,
  fileName: string,
  extraImport: string[]
): string {
  let mockData = '';
  if (sourceFileEntity.moduleDeclarations.length > 0) {
    sourceFileEntity.moduleDeclarations.forEach(value => {
      mockData += generateModuleDeclaration(value, sourceFile, fileName, mockApi, extraImport, sourceFileEntity.importDeclarations) + '\n';
    });
  }
  return mockData;
}

/**
 * get function declarations generate
 * @param sourceFileEntity  all node information in the file
 * @param sourceFile file Text Information
 * @param mockApi file mock text
 * @returns string
 */
function functionDeclarationsGenerate(
  sourceFileEntity: SourceFileEntity,
  sourceFile: SourceFile,
  mockApi: string
): string {
  let mockData = '';
  if (sourceFileEntity.functionDeclarations.size > 0) {
    Array.from(sourceFileEntity.functionDeclarations.keys()).forEach(key => {
      mockData += generateCommonFunction(key, sourceFileEntity.functionDeclarations.get(key), sourceFile, mockApi, true) + '\n';
    });
  }
  return mockData;
}

/**
 * get other declarations generate
 * @param rootName  absolute path to the mock file to be generated
 * @param sourceFileEntity  all node information in the file
 * @param mockFunctionElements  all function element entity
 * @param sourceFile  file Text Information
 * @param mockApi  file mock text
 * @param fileName file name
 * @returns ReturnDataParams
 */
function otherDeclarationsGenerate(
  rootName: string,
  sourceFileEntity: SourceFileEntity,
  mockFunctionElements: Array<MockFunctionElementEntity>,
  sourceFile: SourceFile,
  mockApi: string,
  fileName: string
): ReturnDataParams {
  const data: ReturnDataParams = {
    mockData: '',
    mockFunctionElements: []
  };
  if (
    sourceFileEntity.moduleDeclarations.length === 0 &&
    (fileName.startsWith('ohos_') || fileName.startsWith('system_') || fileName.startsWith('webgl'))
  ) {
    const moduleDeclarationsData = handleModuleDeclarationsNotExist(rootName, fileName, sourceFile, mockApi, mockFunctionElements);
    data.mockData = moduleDeclarationsData.mockData;
    data.mockFunctionElements = moduleDeclarationsData.mockFunctionElements;
  } else {
    const defaultExportClass = getDefaultExportClassDeclaration(sourceFile);
    if (defaultExportClass.length > 0) {
      const mockNameArr = fileName.split('_');
      const mockName = mockNameArr[mockNameArr.length - 1];
      defaultExportClass.forEach(value => {
        data.mockData += generateClassDeclaration(rootName, value, false, mockName, '', sourceFile, false, mockApi) + '\n';
      });
    }
  }
  return data;
}

/**
 * handle Export Declarations
 * @param sourceFileEntity all node information in the file
 * @returns export text info
 */
function handleExportDeclarations(sourceFileEntity: SourceFileEntity): string {
  let mockApi = '';
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
  return mockApi;
}

/**
 * add extra export default expression
 * @param mockApi file mock text
 * @returns export text info
 */
function addExportDefaultExpression(mockApi: string): string {
  const paramIndex = 2;
  const reg = /export\sconst\s.*\s=/g;
  const regDefault = /export\sdefault\s/g;
  const regFunc = /export\sfunction\s/g;
  const results = mockApi.match(reg);
  const resultDefaults = mockApi.match(regDefault);
  const resultFuncs = mockApi.match(regFunc);
  if (results && results.length === 1 && !resultDefaults && !resultFuncs) {
    const arr = results[0].split(' ');
    const moduleName = arr[arr.length - paramIndex];
    mockApi += `\nexport default ${moduleName};`;
  }
  return mockApi;
}

/**
 * generate import definition
 * @param importEntity import entity data
 * @param sourceFileName file name
 * @param heritageClausesArray heritage elements array data
 * @param currentFilePath current file path
 * @param dependsSourceFileList reference Files data
 * @returns string
 */
export function generateImportDeclaration(
  importEntity: ImportElementEntity,
  sourceFileName: string,
  heritageClausesArray: string[],
  currentFilePath: string,
  dependsSourceFileList: SourceFile[]
): string {
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

  const tmpImportPath = importPath.replace(/'/g, '').replace(/"/g, '');
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
  if (!importElements.includes('{') && !importElements.includes('}') && needToAddBrace.includes(importElements)) {
    importElements = `{ ${importElements} }`;
  }
  collectAllLegalImports(importElements);
  return `import ${importElements} from ${importPath}\n`;
}

/**
 * handle module declarations does it exist
 * @param rootName absolute path to the mock file to be generated
 * @param fileName file name
 * @param sourceFile file Text Information
 * @param mockApi file mock text
 * @param mockFunctionElements all function element entity
 * @returns ReturnDataParams
 */
function handleModuleDeclarationsNotExist(
  rootName: string, fileName: string, sourceFile: SourceFile, mockApi: string, mockFunctionElements: Array<MockFunctionElementEntity>
): ReturnDataParams {
  const data: ReturnDataParams = {
    mockData: '',
    mockFunctionElements: mockFunctionElements
  };
  const mockNameArr = fileName.split('_');
  const mockName = mockNameArr[mockNameArr.length - 1];
  const defaultExportClass = getDefaultExportClassDeclaration(sourceFile);
  defaultExportClass.forEach(value => {
    data.mockData += generateClassDeclaration(rootName, value, false, mockName, '', sourceFile, false, mockApi) + '\n';
    data.mockFunctionElements.push({ elementName: value.className, type: 'class' });
  });
  data.mockData += `export function mock${firstCharacterToUppercase(mockName)}() {\n`;
  if (fileName.startsWith('system_')) {
    addToSystemIndexArray({
      filename: fileName,
      mockFunctionName: `mock${firstCharacterToUppercase(mockName)}`
    });
    data.mockData += `global.systemplugin.${mockName} = {`;
    const defaultClass = getDefaultExportClassDeclaration(sourceFile);
    let staticMethodBody = '';
    defaultClass.forEach(value => {
      value.staticMethods.forEach(val => {
        staticMethodBody += generateStaticFunction(val, true, sourceFile, mockApi);
      });
    });
    data.mockData += staticMethodBody;
    data.mockData += '}';
  } else {
    if (!fileName.startsWith('webgl')) {
      addToIndexArray({ fileName: fileName, mockFunctionName: `mock${firstCharacterToUppercase(mockName)}` });
    }
  }
  data.mockData += `\nconst mockModule${firstCharacterToUppercase(mockName)} = {`;
  data.mockFunctionElements.forEach(val => {
    data.mockData += `${val.elementName}: ${val.elementName},`;
  });
  data.mockData += '}\n';
  const isHaveExportDefault = hasExportDefaultKeyword(mockName, sourceFile);
  const mockNameUppercase = firstCharacterToUppercase(mockName);
  data.mockData +=
    isHaveExportDefault ? `return mockModule${mockNameUppercase}\n` : `return mockModule${mockNameUppercase}.${mockNameUppercase}\n`;
  data.mockData += '}';
  return data;
}

/**
 * adapter default export
 * @param importName
 * @returns boolean
 */
function checIsDefaultExportClass(importName: string): boolean {
  const defaultExportClass = ['Context', 'BaseContext', 'ExtensionContext', 'ApplicationContext',
    'ExtensionAbility', 'Ability', 'UIExtensionAbility', 'UIExtensionContext'];
  return defaultExportClass.includes(importName);
}

/**
 * get heritage elements
 * @param sourceFileEntity all node information in the file
 * @param sourceFile file Text Information
 * @returns string[]
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

/**
 * collect reference Files
 * @param sourceFile file Text Information
 * @returns SourceFile[]
 */
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

/**
 * content relatePath to real relatePath
 * @param currentFilePath file name
 * @param contentReferenceRelatePath reference relate Path
 * @returns string
 */
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

/**
 * referenct import to module import
 * @param importEntity import entity data
 * @param currentFilePath current file path data
 * @param dependsSourceFileList reference Files data
 * @returns string
 */
export function referenctImport2ModuleImport(
  importEntity: ImportElementEntity,
  currentFilePath: string,
  dependsSourceFileList: SourceFile[]
): string {
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

/**
 * get import pathName
 * @param importPathSplit import path split to array data
 * @returns string
 */
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

/**
 * get import pathName
 * @param importEntity import entity data
 * @param heritageClausesArray heritage elements array data
 * @returns string
 */
function generateImportElements(importEntity: ImportElementEntity, heritageClausesArray: string[]): string {
  let importElements = importEntity.importElements;
  if (
    !importElements.includes('{') &&
    !importElements.includes('* as') &&
    !heritageClausesArray.includes(importElements) &&
    importEntity.importPath.includes('@ohos')
  ) {
    const tmpArr = importEntity.importPath.split('.');
    const mockModuleName = firstCharacterToUppercase(tmpArr[tmpArr.length - 1].replace('"', '').replace('\'', ''));
    if (importElements === 'observer' && importEntity.importPath.includes('@ohos.arkui.observer')) {
      return `{ mockUiObserver as ${importElements}}`;
    }
    importElements = `{ mock${mockModuleName} }`;
  } else {
    // adapt no rules .d.ts
    if (importElements.trim() === 'AccessibilityExtensionContext, { AccessibilityElement }') {
      importElements = '{ AccessibilityExtensionContext, AccessibilityElement }';
    } else if (importElements.trim() === '{ image }') {
      importElements = '{ mockImage as image }';
    }
  }
  return importElements;
}
