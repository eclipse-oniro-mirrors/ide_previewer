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

import { SyntaxKind } from 'typescript';
import type { SourceFile } from 'typescript';
import type { PropertySignatureEntity } from '../declaration-node/propertySignatureDeclaration';
import {
  checkIsGenericSymbol,
  getCallbackStatement,
  getTheRealReferenceFromImport,
  getWarnConsole,
  propertyTypeWhiteList,
  paramsTypeStart,
  getEnumReturnValue
} from './generateCommonUtil';
import { allEnumMap } from '../common/commonUtils';

/**
 * generate interface signature property
 * @param rootName
 * @param propertySignature
 * @param sourceFile
 * @returns
 */
export function generatePropertySignatureDeclaration(
  rootName: string,
  propertySignature: PropertySignatureEntity,
  sourceFile: SourceFile,
  mockApi: string
): string {
  let propertySignatureBody = '';
  if (propertySignature.kind === SyntaxKind.FunctionType) {
    propertySignatureBody += `${propertySignature.propertyName}: function(...args) {`;
    propertySignatureBody += getWarnConsole(rootName, propertySignature.propertyName);
    propertySignatureBody += getCallbackStatement(mockApi);
    propertySignatureBody += '},\n';
  } else {
    if (
      (propertySignature.propertyTypeName.startsWith('{') ||
      propertySignature.propertyTypeName.startsWith('Record<') ||
      propertySignature.propertyTypeName.startsWith('Object') ||
      propertySignature.propertyTypeName.startsWith('object')) &&
      !propertySignature.propertyTypeName.endsWith(']')
    ) {
      propertySignatureBody = `${propertySignature.propertyName}: {},`;
    } else if (propertySignature.kind === SyntaxKind.TypeReference) {
      propertySignatureBody = generatePropertySignatureForTypeReference(propertySignature, sourceFile);
    } else if (propertySignature.kind === SyntaxKind.NumberKeyword) {
      propertySignatureBody = `${propertySignature.propertyName}: 0,`;
    } else if (propertySignature.kind === SyntaxKind.StringKeyword) {
      propertySignatureBody = `${propertySignature.propertyName}: '[PC Preview] unknown ${propertySignature.propertyName}',`;
    } else if (propertySignature.kind === SyntaxKind.BooleanKeyword) {
      propertySignatureBody = `${propertySignature.propertyName}: true,`;
    } else if (propertySignature.kind === SyntaxKind.UnionType) {
      propertySignatureBody = generatePropertySignatureForUnionType(propertySignature, sourceFile);
    } else if (propertySignature.kind === SyntaxKind.ArrayType) {
      propertySignatureBody = `${propertySignature.propertyName}: [],`;
    } else {
      propertySignatureBody = `${propertySignature.propertyName}: '[PC Preview] unknown ${propertySignature.propertyName}',`;
    }
  }
  return propertySignatureBody;
}

/**
 * generate interface signature property for TypeReference
 * @param propertySignature
 * @param sourceFile
 * @returns
 */
function generatePropertySignatureForTypeReference(
  propertySignature: PropertySignatureEntity,
  sourceFile: SourceFile
): string {
  let propertySignatureBody = '';
  if (allEnumMap.has(propertySignature.propertyTypeName)) {
    const resultValue = getEnumReturnValue(propertySignatureBody, propertySignature.propertyTypeName, sourceFile);
    if (resultValue !== '') {
      return `${propertySignature.propertyName}: ${resultValue},`;
    }
  }
  if (propertySignature.propertyTypeName.startsWith('Array')) {
    propertySignatureBody = `${propertySignature.propertyName}: [],`;
  } else if (propertySignature.propertyTypeName.startsWith('Map')) {
    propertySignatureBody = `${propertySignature.propertyName}: {key: {}},`;
  } else if (
    propertySignature.propertyTypeName === 'string' ||
    checkIsGenericSymbol(propertySignature.propertyTypeName) ||
    propertySignature.propertyTypeName === 'bool' ||
    propertySignature.propertyTypeName === 'Data'
  ) {
    propertySignatureBody = `${propertySignature.propertyName}: '[PC Preview] unknown ${propertySignature.propertyName}',`;
  } else if (propertySignature.propertyTypeName === 'IlluminateType') {
    propertySignatureBody = `${propertySignature.propertyName}: '',`;
  } else {
    if (propertySignature.propertyTypeName.includes('<')) {
      propertySignatureBody = handlepropertyTypeNameBody(propertySignature, sourceFile);
    } else {
      if (propertyTypeWhiteList(propertySignature.propertyTypeName, sourceFile) === propertySignature.propertyTypeName) {
        propertySignatureBody = `${propertySignature.propertyName}: ${getTheRealReferenceFromImport(sourceFile, propertySignature.propertyTypeName)},`;
      } else {
        propertySignatureBody = `${propertySignature.propertyName}: ${propertyTypeWhiteList(propertySignature.propertyTypeName, sourceFile)},`;
      }
    }
  }
  return propertySignatureBody;
}

/**
 * generate interface signature property for UnionType
 * @param propertySignature
 * @param sourceFile
 * @returns
 */
function generatePropertySignatureForUnionType(
  propertySignature: PropertySignatureEntity,
  sourceFile: SourceFile
): string {
  let propertySignatureBody = '';
  let unionFirstElement = propertySignature.propertyTypeName.split('|')[0].trim();
  if (unionFirstElement.includes('[]') || unionFirstElement.startsWith('[') || unionFirstElement.endsWith(']')) {
    unionFirstElement = '[]';
  }
  if (unionFirstElement.startsWith('"') || unionFirstElement.startsWith("'")) {
    propertySignatureBody = `${propertySignature.propertyName}: ${unionFirstElement},`;
  } else if (unionFirstElement === 'string') {
    propertySignatureBody = `${propertySignature.propertyName}: '[PC Preview] unknown ${propertySignature.propertyName}',`;
  } else if (unionFirstElement === 'number') {
    propertySignatureBody = `${propertySignature.propertyName}: 0,`;
  } else if (unionFirstElement === 'boolean') {
    propertySignatureBody = `${propertySignature.propertyName}: true,`;
  } else if (unionFirstElement === 'Uint8Array') {
    propertySignatureBody = `${propertySignature.propertyName}: new ${unionFirstElement}(),`;
  } else {
    let element = unionFirstElement;
    const returnElement = propertyTypeWhiteList(unionFirstElement, sourceFile);
    if (element === 'HTMLCanvasElement') {
      element = `'[PC Preview] unknown ${propertySignature.propertyName}'`;
    } else if (element === 'WebGLActiveInfo') {
      element = '{size: \'[PC Preview] unknown GLint\', type: 0, name: \'[PC Preview] unknown name\'}';
    } else if (element.startsWith('Array')) {
      element = '[]';
    } else if (returnElement === unionFirstElement) {
      element = getTheRealReferenceFromImport(sourceFile, unionFirstElement);
    } else if (returnElement.toString().includes(`${unionFirstElement}.`)) {
      element = returnElement.toString();
    }
    propertySignatureBody = `${propertySignature.propertyName}: ${element},`;
  }
  return propertySignatureBody;
}

/**
 * generate interface signature property for TypeReference
 * @param propertySignature
 * @param sourceFile
 * @returns
 */
function handlepropertyTypeNameBody(propertySignature: PropertySignatureEntity, sourceFile: SourceFile): string {
  let propertySignatureBody = '';
  if (
    propertySignature.propertyTypeName.startsWith('AsyncCallback') ||
    propertySignature.propertyTypeName.startsWith('Callback')
  ) {
    propertySignatureBody = `${propertySignature.propertyName}: ()=>{},`;
  } else {
    const preSplit = propertySignature.propertyTypeName.split('<');
    const genericArg = preSplit[preSplit.length - 1].split('>')[0];
    if (genericArg.includes(',')) {
      return `${propertySignature.propertyName}: {},`;
    }
    let sourceFileContent = sourceFile.text;
    const removeNoteRegx = /\/\*[\s\S]*?\*\//g;
    sourceFileContent = sourceFileContent.replace(removeNoteRegx, '');
    const regex = new RegExp(`\\s${genericArg}\\s({|=|extends)`);
    const results = sourceFileContent.match(regex);
    if (results) {
      propertySignatureBody = `${propertySignature.propertyName}: ${genericArg},`;
    } else {
      let getPropertyTypeName = null;
      Object.keys(paramsTypeStart).forEach(key => {
        if (genericArg.startsWith(key)) {
          getPropertyTypeName =
            paramsTypeStart[key] === '[PC Preview] unknown type'
              ? `'${paramsTypeStart[key]}'`
              : `${paramsTypeStart[key]}`;
        }
      });
      propertySignatureBody = `${propertySignature.propertyName}: ${
        getPropertyTypeName ?? '\'[PC Preview] unknown type\''
      },`;
    }
  }
  return propertySignatureBody;
}
