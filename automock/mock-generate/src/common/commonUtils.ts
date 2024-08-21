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
import type {
  CallSignatureDeclaration, ComputedPropertyName, FunctionDeclaration, Identifier, MethodDeclaration,
  MethodSignature, ModifiersArray, ModuleDeclaration, NodeArray, ParameterDeclaration, PropertyName, SourceFile
} from 'typescript';
import {
  isClassDeclaration, isComputedPropertyName, isIdentifier, isModuleBlock, isModuleDeclaration, isPrivateIdentifier
} from 'typescript';
import fs from 'fs';
import ts from 'typescript';
import type { ImportElementEntity } from '../declaration-node/importAndExportDeclaration';
import { collectAllKitFiles } from './kitUtils';

const paramIndex = 2;
const allLegalImports = new Set<string>();
const fileNameList = new Set<string>();
const allClassSet = new Set<string>();

export const dtsFileList: Array<string> = [];

/**
 * get all legal imports
 * @returns
 */
export function getAllLegalImports(): Set<string> {
  return allLegalImports;
}

/**
 * get all legal imports
 * @param element
 */
export function collectAllLegalImports(element: string): void {
  allLegalImports.add(element);
}

/**
 * collect all mock js file path
 * @returns
 */
export function getAllFileNameList(): Set<string> {
  return fileNameList;
}

/**
 * collect all file name
 */
export function collectAllFileName(filePath: string): void {
  const fullFileName = path.basename(filePath);
  let fileName = '';
  if (fullFileName.endsWith('d.ts')) {
    fileName = fullFileName.split('.d.ts')[0];
  } else if (fullFileName.endsWith('d.ets')) {
    fileName = fullFileName.split('.d.ets')[0];
  }

  let outputFileName = '';
  if (fileName.includes('@')) {
    outputFileName = fileName.split('@')[1].replace(/\./g, '_');
  } else {
    outputFileName = fileName;
  }
  fileNameList.add(outputFileName);
}

/**
 * get all class name set
 * @returns
 */
export function getClassNameSet(): Set<string> {
  return allClassSet;
}

/**
 * get all class declaration
 * @param sourceFile
 * @returns
 */
export function getAllClassDeclaration(sourceFile: SourceFile): Set<string> {
  sourceFile.forEachChild(node => {
    if (isClassDeclaration(node)) {
      if (node.name !== undefined) {
        allClassSet.add(node.name.escapedText.toString());
      }
    } else if (isModuleDeclaration(node)) {
      const moduleDeclaration = node as ModuleDeclaration;
      const moduleBody = moduleDeclaration.body;
      getIsModuleDeclaration(moduleBody);
    }
  });
  return allClassSet;
}

/**
 * get module class declaration
 * @param moduleBody
 * @returns
 */
function getIsModuleDeclaration(moduleBody: ts.ModuleBody): void {
  if (moduleBody !== undefined && isModuleBlock(moduleBody)) {
    moduleBody.statements.forEach(value => {
      if (isClassDeclaration(value) && value.name !== undefined) {
        allClassSet.add(firstCharacterToUppercase(value.name?.escapedText.toString()));
      }
    });
  }
}

/**
 * get keywords
 * @param modifiers
 * @returns
 */
export function getModifiers(modifiers: ModifiersArray): Array<number> {
  const modifiersArray: Array<number> = [];
  modifiers.forEach(value => modifiersArray.push(value.kind));
  return modifiersArray;
}

/**
 * get property name
 * @param node property node
 * @param sourceFile
 * @returns
 */
export function getPropertyName(node: PropertyName, sourceFile: SourceFile): string {
  let propertyName = '';
  if (isIdentifier(node) || isPrivateIdentifier(node)) {
    const newNameNode = node as Identifier;
    propertyName = newNameNode.escapedText.toString();
  } else if (isComputedPropertyName(node)) {
    const newNameNode = node as ComputedPropertyName;
    propertyName = sourceFile.text.substring(newNameNode.expression.pos, newNameNode.expression.end).trim();
  } else {
    propertyName = sourceFile.text.substring(node.pos, node.end).trim();
  }
  return propertyName;
}

/**
 * get parameter declaration
 * @param parameter
 * @param sourceFile
 * @returns
 */
export function getParameter(parameter: ParameterDeclaration, sourceFile: SourceFile): ParameterEntity {
  let paramName = '';
  let paramTypeString = '';
  const paramTypeKind = parameter.type?.kind === undefined ? -1 : parameter.type.kind;
  if (isIdentifier(parameter.name)) {
    paramName = parameter.name.escapedText === undefined ? '' : parameter.name.escapedText.toString();
  } else {
    const start = parameter.name.pos === undefined ? 0 : parameter.name.pos;
    const end = parameter.name.end === undefined ? 0 : parameter.name.end;
    paramName = sourceFile.text.substring(start, end).trim();
  }

  const start = parameter.type?.pos === undefined ? 0 : parameter.type.pos;
  const end = parameter.type?.end === undefined ? 0 : parameter.type.end;
  paramTypeString = sourceFile.text.substring(start, end).trim();
  return {
    paramName: paramName,
    paramTypeString: paramTypeString,
    paramTypeKind: paramTypeKind
  };
}

/**
 * get method or function return info
 * @param node
 * @param sourceFile
 * @returns
 */
export function getFunctionAndMethodReturnInfo(
  node: FunctionDeclaration | MethodDeclaration | MethodSignature | CallSignatureDeclaration,
  sourceFile: SourceFile
): ReturnTypeEntity {
  const returnInfo = { returnKindName: '', returnKind: -1 };
  if (node.type !== undefined) {
    const start = node.type.pos === undefined ? 0 : node.type.pos;
    const end = node.type.end === undefined ? 0 : node.type.end;
    returnInfo.returnKindName = sourceFile.text.substring(start, end).trim();
    returnInfo.returnKind = node.type.kind;
  }
  return returnInfo;
}

/**
 * get export modifiers
 * @param modifiers
 * @returns
 */
export function getExportKeyword(modifiers: ModifiersArray): Array<number> {
  const modifiersArray: Array<number> = [];
  modifiers.forEach(value => {
    modifiersArray.push(value.kind);
  });
  return modifiersArray;
}

/**
 *
 * @param str first letter capitalization
 * @returns
 */
export function firstCharacterToUppercase(str: string): string {
  return str.slice(0, 1).toUpperCase() + str.slice(1);
}

/**
 * parameters entity
 */
export interface ParameterEntity {
  paramName: string,
  paramTypeString: string,
  paramTypeKind: number
}

/**
 * return type entity
 */
export interface ReturnTypeEntity {
  returnKindName: string,
  returnKind: number
}

/**
 * Get OpenHarmony project dir
 * @return project dir
 */

export function getProjectDir(): string {
  const apiInputPath = process.argv[paramIndex];
  const privateInterface = path.join('vendor', 'huawei', 'interface', 'hmscore_sdk_js', 'api');
  const openInterface = path.join('interface', 'sdk-js', 'api');
  if (apiInputPath.indexOf(openInterface) > -1) {
    return apiInputPath.replace(`${path.sep}${openInterface}`, '');
  } else {
    return apiInputPath.replace(`${path.sep}${privateInterface}`, '');
  }
}

/**
 * return interface api dir in OpenHarmony
 */
export function getOhosInterfacesDir(): string {
  return path.join(getProjectDir(), 'interface', 'sdk-js', 'api');
}

/**
 * return interface api root path
 * @returns apiInputPath
 */
export function getApiInputPath(): string {
  return process.argv[paramIndex];
}

/**
 * return OpenHarmony file path dependent on by HarmonyOs
 * @param importPath path of depend imported
 * @param sourceFile sourceFile of current file
 * @returns dependsFilePath
 */
export function findOhosDependFile(importPath: string, sourceFile: SourceFile): string {
  const interFaceDir = getOhosInterfacesDir();
  const tmpImportPath = importPath.replace(/'/g, '').replace('.d.ts', '').replace('.d.ets', '');
  const sourceFileDir = path.dirname(sourceFile.fileName);
  let dependsFilePath: string;
  if (tmpImportPath.startsWith('./')) {
    const subIndex = 2;
    dependsFilePath = path.join(sourceFileDir, tmpImportPath.substring(subIndex));
  } else if (tmpImportPath.startsWith('../')) {
    const backSymbolList = tmpImportPath.split('/').filter(step => step === '..');
    dependsFilePath = [
      ...sourceFileDir.split(path.sep).slice(0, -backSymbolList.length),
      ...tmpImportPath.split('/').filter(step => step !== '..')
    ].join(path.sep);
  } else if (tmpImportPath.startsWith('@ohos.inner.')) {
    const pathSteps = tmpImportPath.replace(/@ohos\.inner\./g, '').split('.');
    for (let i = 0; i < pathSteps.length; i++) {
      const tmpInterFaceDir = path.join(interFaceDir, ...pathSteps.slice(0, i), pathSteps.slice(i).join('.'));
      if (fs.existsSync(tmpInterFaceDir + '.d.ts')) {
        return tmpInterFaceDir + '.d.ts';
      }

      if (fs.existsSync(tmpInterFaceDir + '.d.ets')) {
        return tmpInterFaceDir + '.d.ets';
      }
    }
  } else if (tmpImportPath.startsWith('@ohos.')) {
    dependsFilePath = path.join(getOhosInterfacesDir(), tmpImportPath);
  }

  if (fs.existsSync(dependsFilePath + '.d.ts')) {
    return dependsFilePath + '.d.ts';
  }

  if (fs.existsSync(dependsFilePath + '.d.ets')) {
    return dependsFilePath + '.d.ets';
  }

  console.warn(`Cannot find module '${importPath}'`);
  return '';
}

/**
 * Determine if the file is a openHarmony interface file
 * @param path: interface file path
 * @returns
 */
export function isOhosInterface(path: string): boolean {
  return path.startsWith(getOhosInterfacesDir());
}

/**
 * reutn js-sdk root folder full path
 * @returns
 */
export function getJsSdkDir(): string {
  let sdkJsDir = process.argv[paramIndex].split(path.sep).slice(0, -1).join(path.sep);
  sdkJsDir += sdkJsDir.endsWith(path.sep) ? '' : path.sep;
  return sdkJsDir;
}

/**
 * Determine whether the object has been imported
 * @param importDeclarations imported Declaration list in current file
 * @param typeName Object being inspected
 * @returns 
 */
export function hasBeenImported(importDeclarations: ImportElementEntity[], typeName: string): boolean {
  if (!typeName.trim()) {
    return true;
  }
  if (isFirstCharLowerCase(typeName)) {
    return true;
  }
  return importDeclarations.some(importDeclaration => {
    if (importDeclaration.importElements.includes(typeName) && importDeclaration.importPath.includes('./')) {
      return true;
    }
    return false;
  });
}

/**
 * Determine whether the first character in a string is a lowercase letter
 * @param str target string
 * @returns 
 */
function isFirstCharLowerCase(str: string): boolean {
  const lowerCaseFirstChar = str[0].toLowerCase();
  return str[0] === lowerCaseFirstChar;
}

export const specialFiles = [
  '@internal/component/ets/common.d.ts',
  '@internal/component/ets/units.d.ts',
  '@internal/component/ets/common_ts_ets_api.d.ts',
  '@internal/component/ets/enums.d.ts',
  '@internal/component/ets/alert_dialog.d.ts',
  '@internal/component/ets/ability_component.d.ts',
  '@internal/component/ets/rich_editor.d.ts',
  '@internal/component/ets/symbolglyph.d.ts',
  '@internal/component/ets/button.d.ts',
  '@internal/component/ets/nav_destination.d.ts',
  '@internal/component/ets/navigation.d.ts',
  '@internal/component/ets/text_common.d.ts',
  '@internal/component/ets/styled_string.d.ts'
];

export const specialType = [
  'Storage',
  'File',
  'ChildProcess',
  'Cipher',
  'Sensor',
  'Authenticator'
];

export const specialClassName = [
  'Want',
  'Configuration',
  'InputMethodExtensionContext'
];

/**
 * get add kit file map
 * @param apiInputPath api input path
 * @returns
 */
export function generateKitMap(apiInputPath: string): void {
  const kitPath = path.join(apiInputPath, '../', 'kits');
  if (!fs.existsSync(kitPath)) {
    throw new Error(`${kitPath} does not exist.`);
  }
  collectAllKitFiles(kitPath);
}

export interface DependencyListParams {
  dependency: Array<string>,
  export: string
}

export interface DependencyParams {
  [key: string]: DependencyListParams
}

// dependence on collecting files
export const DEPENDENCY_LIST: DependencyParams = {};

/**
 * generated depend.json
 */
export function generateDependJsonFile(): void {
  const dependInfoPath = path.join(__dirname, '../../../runtime/main/extend/systemplugin/depend.json');
  fs.writeFileSync(dependInfoPath, JSON.stringify(DEPENDENCY_LIST, null, 2), 'utf-8');
}

/**
 * generated MyComponent.js
 * 
 * @param outDir generated file root directory
 */
export function generateMyComponent(outDir: string): void {
  fs.writeFileSync(
    path.join(outDir, 'MyComponent.js'),
    'class MyComponent {}\nexport { MyComponent };'
  );
}

// initialize all variables in the file
export let INITVARIABLE = '';

/**
 * set initialize variable
 * 
 * @param value variable name
 */
export function setInitVariable(value?: string): void {
  if (value) {
    if (!INITVARIABLE.includes(`let ${value} = {};`)) {
      INITVARIABLE += `let ${value} = {};\n`;
    }
  } else {
    INITVARIABLE = '';
  }
}

/**
 * get all initialize variable
 * @returns string
 */
export function getInitVariable(): string {
  return INITVARIABLE;
}