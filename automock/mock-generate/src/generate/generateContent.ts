/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
  DECLARES,
  IGNORE_REFERENCES,
  importDeclarationFiles,
  KeyValueTypes,
  mockBufferMap,
  MockedFileMap,
  TSTypes,
  specialOverloadedFunctionArr,
  callbackError, ClassNotInEts,
  undefinedTypes,
  windowDeclaration
} from '../common/constants';
import {
  Declare,
  KeyValue,
  ReferenceFindResult,
  Members,
  MockBuffer,
  OverloadedFunctionType,
  CallbackParamMockData
} from '../types';
import { generateKeyValue, isMockedDeclarations } from '../common/commonUtils';

/**
 * 生成文件内容
 * @param mockBuffer mock信息
 * @param members 文件根节点的成员
 * @returns
 */
export function generateContent(mockBuffer: MockBuffer, members: Members): string {
  const membersContent: string[] = [];
  Object.keys(members).forEach(memberKey => {
    if (memberKey === 'default') {
      return;
    }
    const keyValue = members[memberKey];
    if (!keyValue.isNeedMock) {
      return;
    }
    if (keyValue.type === KeyValueTypes.IMPORT) {
      return;
    }
    if (keyValue.isGlobalDeclare && windowDeclaration.includes(memberKey)) {
      membersContent.push(`export const ${memberKey}=global.${memberKey}_temp;`);
    } else if (keyValue.isGlobalDeclare) {
      membersContent.push(`export const ${memberKey}=global.${memberKey};`);
    } else {
      const memberBody = handleKeyValue(memberKey, keyValue, mockBuffer, [], keyValue, keyValue.property);
      membersContent.push(`export const ${memberKey} = ${memberBody};`);
    }
    if (keyValue.isDefault) {
      const exportDefaultStr = `export default ${keyValue.key}`;
      membersContent.push(exportDefaultStr);
    }
  });
  return membersContent.join('\n');
}

/**
 * 处理KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleKeyValue(
  key: string,
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  property: KeyValue
): string {
  if (keyValue.value !== undefined) {
    return keyValue.value;
  }
  if (new Set<KeyValueTypes>([
    KeyValueTypes.CLASS,
    KeyValueTypes.MODULE,
    KeyValueTypes.INTERFACE
  ]).has(keyValue.type) && kvPath.includes(keyValue)) {
    if (keyValue.isGlobalDeclare) {
      return `global.${keyValue.key}`;
    }
    if (keyValue.parent.isGlobalDeclare) {
      return `global.${keyValue.parent.key}_temp.${keyValue.key}`;
    }
    return 'this';
  } else {
    kvPath = kvPath.concat([keyValue]);
  }

  return mockKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
}

/**
 * 根据KV节点生成文件内容
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function mockKeyValue(
  key: string,
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  property: KeyValue
): string {
  let value: string;
  switch (keyValue.type) {
    case KeyValueTypes.CLASS: {
      value = handleClassKeyValue(keyValue, mockBuffer, kvPath, rootKeyValue);
      break;
    }
    case KeyValueTypes.EXPORT: {
      value = handleExportKeyValue(keyValue, mockBuffer);
      break;
    }
    case KeyValueTypes.FILE: {
      value = handleFileKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.FUNCTION: {
      value = handleFunctionKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue);
      break;
    }
    case KeyValueTypes.IMPORT: {
      value = handleImportKeyValue(keyValue, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.INTERSECTION: {
      value = handleIntersectionKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue);
      break;
    }
    case KeyValueTypes.MODULE: {
      value = handleModuleKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.INTERFACE: {
      value = handleInterfaceKeyValue(keyValue, mockBuffer, kvPath, rootKeyValue);
      break;
    }
    case KeyValueTypes.VALUE: {
      value = handleValueKeyValue(keyValue);
      break;
    }
    case KeyValueTypes.VARIABLE: {
      value = handleVariableKeyValue(keyValue, mockBuffer, kvPath, rootKeyValue);
      break;
    }
    case KeyValueTypes.PROPERTY: {
      value = handlePropertyKeyValue(keyValue, mockBuffer, kvPath, rootKeyValue);
      break;
    }
    case KeyValueTypes.REFERENCE: {
      value = handleReferenceKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.ENUM: {
      value = handleEnumKeyValue(keyValue, mockBuffer, kvPath, rootKeyValue);
      break;
    }
    case KeyValueTypes.EXPRESSION: {
      value = handleExpressionKeyValue(keyValue, mockBuffer, kvPath, rootKeyValue);
      break;
    }
  }
  keyValue.value = value;
  return value;
}

/**
 * 处理class KV节点
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleClassKeyValue(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const memberLines: string[] = [];
  const dynamicProperties: string[] = ['this.isAutoMock=true'];

  if (keyValue.heritage) {
    handleHeritage(keyValue, mockBuffer, kvPath.concat([keyValue.heritage]), rootKeyValue);
  }

  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    let elementName = memberKey;

    if (memberKeyValue.type === KeyValueTypes.EXPRESSION) {
      memberKeyValue.key = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
      memberKeyValue.type = KeyValueTypes.FUNCTION;
      memberKeyValue.value = undefined;
      elementName = memberKeyValue.key;
    }
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    if (memberKeyValue.type === KeyValueTypes.FUNCTION) {
      if (memberKeyValue.members.IterableIterator) {
        memberLines.push(`*${elementName}${value.replace(/^function\*?/, '')}`);
      } else {
        memberLines.push(`${memberKeyValue.isStatic ? 'static ' : ''}${elementName}${value.replace(/^function\*?/, '')}`);
      }
    } else {
      if (memberKeyValue.isStatic) {
        memberLines.push(`static get ${elementName}(){return ${value.replace(/^function\*?/, '')}}`);
      } else {
        dynamicProperties.push(`this.${elementName} = ${value}`);
      }
    }
  });

  return `class {constructor() {\n${dynamicProperties.join(';\n')}\n}\n${memberLines.join(';\n')}\n}`;
}

/**
 * 处理继承
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleHeritage(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): void {
  const keyValueInfo = findKeyValueDefined(keyValue.heritage.key, keyValue.heritage, mockBuffer, kvPath, rootKeyValue, keyValue.heritage.property);
  const defKeyValue = keyValueInfo.keyValue;
  const defMockBuffer = keyValueInfo.mockBuffer;
  handleKeyValue(defKeyValue.key, defKeyValue, defMockBuffer, kvPath, rootKeyValue, defKeyValue.property);

  Object.keys(defKeyValue.members).forEach(memberKey => {
    const memberKeyValue = Object.assign({}, defKeyValue.members[memberKey]);
    memberKeyValue.isMocked = false;
    if (memberKeyValue.type === KeyValueTypes.EXPRESSION) {
      memberKeyValue.key = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
      memberKeyValue.type = KeyValueTypes.PROPERTY;
    }
    if (keyValue.members[memberKeyValue.key]) {
      return;
    }
    keyValue.members[memberKeyValue.key] = memberKeyValue;
  });
}

/**
 * 处理export KV节点
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @returns
 */
function handleExportKeyValue(
  keyValue: KeyValue,
  mockBuffer: MockBuffer
): string {
  return `export * from './${path.relative(path.dirname(mockBuffer.mockedFilePath), keyValue.key)}';`;
}

/**
 * 处理file KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleFileKeyValue(
  key: string,
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  property: KeyValue
): string {
  if (property) {
    const propertyKeyValue = keyValue.members[property.key];
    if (propertyKeyValue) {
      return handleKeyValue(property.key, propertyKeyValue, mockBuffer, kvPath, rootKeyValue, property);
    } else {
      console.warn(`Not found ${property.key} from ${key} in file ${mockBuffer.rawFilePath}`);
    }
  }
  return '\'\'';
}

/**
 * 处理function KV节点
 * @param key
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleFunctionKeyValue(
  key: string,
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const memberKey = 'IterableIterator';
  const memberKeyValue = keyValue.members[memberKey];
  if (memberKeyValue) {
    return handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
  }

  const sameFuncList: KeyValue[] = [];
  sameFuncList.push(keyValue);

  keyValue.sameName.forEach(sameFunction => {
    sameFuncList.push(sameFunction);
  });
  return handleSameFunctions(key, sameFuncList, mockBuffer, kvPath, rootKeyValue);
}

/**
 * 处理import KV节点
 * @param keyValue KV节点
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleImportKeyValue(
  keyValue: KeyValue,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  property: KeyValue
): string {
  const importedMockBuffer = mockBufferMap.get(MockedFileMap.get(keyValue.importedModulePath));
  const importedRootKeyValue = importedMockBuffer.contents;
  if (keyValue.isImportDefault) {
    const defaultKeyValue = importedRootKeyValue.members.default;
    if (!defaultKeyValue) {
      console.warn(`The file: ${importedMockBuffer.rawFilePath} does not contain the default export.`);
      return `'The file: ${importedMockBuffer.rawFilePath} does not contain the default export.'`;
    }
    return handleKeyValue(defaultKeyValue.key, defaultKeyValue, importedMockBuffer, kvPath, rootKeyValue, property);
  }

  const targetKeyValue = importedRootKeyValue.members[keyValue.rawName ?? keyValue.key];
  if (!targetKeyValue) {
    console.warn(`The ${keyValue.rawName ?? keyValue.key} does not export from ${MockedFileMap.get(keyValue.importedModulePath)}.`);
    return '"Unknown type"';
  }
  return handleKeyValue(targetKeyValue.key, targetKeyValue, importedMockBuffer, kvPath, rootKeyValue, property);
}

/**
 * 处理intersection KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleIntersectionKeyValue(
  key: string,
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const params: string[] = [];
  Object.keys(keyValue.methodParams).forEach(param => {
    const paramKeyValue = keyValue.methodParams[param];
    const value = handleKeyValue(param, paramKeyValue, mockBuffer, kvPath, rootKeyValue, paramKeyValue.property);
    // 因为rollup在编译时，会将this编译成undefined，导致有运行时报错，因此需要打个补丁
    params.push(`(${value}) || {}`);
  });
  return `${key}(${params.join(', ')})`;
}

/**
 * 处理module KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleModuleKeyValue(
  key: string,
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  property?: KeyValue
): string {
  const memberLines: string[] = [];
  if (property) {
    const propertyKeyValue = keyValue.members[property.key];
    if (propertyKeyValue) {
      return handleKeyValue(property.key, propertyKeyValue, mockBuffer, kvPath, rootKeyValue, propertyKeyValue.property);
    } else {
      console.warn(`Not found ${property.key} from ${key} in file ${mockBuffer.rawFilePath}`);
    }
  }
  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    if (!keyValue.isGlobalDeclare && !memberKeyValue.isNeedMock) {
      return;
    }
    const value = `${memberKeyValue.key}: ${handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property)}`;
    memberLines.push(value);
  });
  return `{\n${memberLines.join(',\n')}\n}`;
}

/**
 * 处理interface KV节点
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleInterfaceKeyValue(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const memberLines: string[] = ['isAutoMock: true'];

  if (keyValue.heritage) {
    handleHeritage(keyValue, mockBuffer, kvPath.concat([keyValue.heritage]), rootKeyValue);
  }
  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    const value = `${memberKeyValue.key}: ${handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property)}`;
    memberLines.push(value);
  });
  return `{\n${memberLines.join(',\n')}\n}`;
}

/**
 * 处理value KV节点
 * @param keyValue KV节点
 * @returns
 */
function handleValueKeyValue(
  keyValue: KeyValue
): string {
  return keyValue.key;
}

/**
 * 处理variable KV节点
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleVariableKeyValue(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const memberLines: string[] = [];
  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberLines.push(value);
  });
  return memberLines.join(',\n');
}

/**
 * 查找reference KV节点定义位置
 * @param key KV节点key值
 * @param targetKeyValue 需要查找的reference KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findKeyValueDefined(
  key: string,
  targetKeyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  property: KeyValue
): ReferenceFindResult {
  let keyValueInfo: ReferenceFindResult;

  // 在js库中找
  keyValueInfo = findInLibs(key, targetKeyValue, mockBuffer, kvPath, rootKeyValue);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 在当前文件中找
  keyValueInfo = findInCurrentFile(key, targetKeyValue, targetKeyValue.parent, mockBuffer, kvPath, rootKeyValue, property);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 在全局定义中找
  keyValueInfo = findInDeclares(key);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 在TS内置类型中找
  keyValueInfo = findTSTypes(key, targetKeyValue, mockBuffer, kvPath, rootKeyValue);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 在所有文件中找
  keyValueInfo = findInAllFiles(key, property);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 全局都未找到定义的类型，做特殊处理
  if (Object.keys(undefinedTypes).includes(key)) {
    const keyValue: KeyValue = generateKeyValue(key, KeyValueTypes.VALUE);
    keyValue.value = undefinedTypes[key];
    return { keyValue, mockBuffer };
  }

  const keyValuePath: string[] = getKeyValuePath(targetKeyValue);
  const fromFilePath = MockedFileMap.get(keyValuePath[0]).replace(/\\/, '/');
  const value: string = `'Cannot find type definition for ${keyValuePath.slice(1).join('->')} from file ${fromFilePath}'`;
  console.warn(value);
  const keyValue: KeyValue = generateKeyValue(key, KeyValueTypes.VALUE);
  keyValue.value = value;
  return { keyValue, mockBuffer };
}

/**
 * 在 typescript 内置类型库中查找类型定义
 * @param key KV节点key值
 * @param targetKeyValue 需要查找的reference KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function findTSTypes(
  key: string,
  targetKeyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): ReferenceFindResult {
  const paramsContent: string[] = [];
  Object.keys(targetKeyValue.typeParameters).forEach(typeParameter => {
    const typeParameterKeyValue: KeyValue = targetKeyValue.typeParameters[typeParameter];
    const paramContent: string = handleKeyValue(typeParameter, typeParameterKeyValue, mockBuffer, kvPath, rootKeyValue, typeParameterKeyValue.property);
    paramsContent.push(paramContent);
  });
  if (!TSTypes[key]) {
    return undefined;
  }
  const keyValue: KeyValue = generateKeyValue(key, KeyValueTypes.VALUE);
  keyValue.value = TSTypes[key](paramsContent.join(', '));
  return { keyValue, mockBuffer };
}

/**
 * 处理函数参数
 * @param params 参数成员
 * @param mockBuffer 当前文件mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleParams(
  params: Members,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const contents: string[] = [];
  Object.keys(params).forEach(key => {
    const paramKeyValue = params[key];
    contents.push(handleKeyValue(paramKeyValue.key, paramKeyValue, mockBuffer, kvPath, rootKeyValue, paramKeyValue.property));
  });
  return contents.join(', ');
}

/**
 * 在 typescript 内置类型库中查找类型定义
 * @param key KV节点key值
 * @param targetKeyValue 需要查找的reference KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function findInLibs(
  key: string,
  targetKeyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): ReferenceFindResult {
  if (key === 'globalThis') {
    const globalThisKeyValue = generateKeyValue(key, KeyValueTypes.VALUE);
    return { keyValue: globalThisKeyValue, mockBuffer };
  }
  if (ClassNotInEts.has(key) || !global[key]) {
    return undefined;
  }
  if (key === 'Symbol') {
    return {
      keyValue: generateKeyValue('[Symbol.iterator]', KeyValueTypes.VALUE),
      mockBuffer
    };
  }
  switch (typeof global[key]) {
    case 'bigint': {
      break;
    }
    case 'boolean': {
      break;
    }
    case 'function': {
      return findInLibFunction(key, targetKeyValue, mockBuffer, kvPath, rootKeyValue);
    }
    case 'number': {
      break;
    }
    case 'object': {
      break;
    }
    case 'string': {
      break;
    }
    case 'symbol': {
      break;
    }
    case 'undefined': {
      break;
    }
  }
  return undefined;
}

/**
 * 在库函数中查找类型定义
 * @param key KV节点key值
 * @param targetKeyValue 需要查找的reference KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function findInLibFunction(
  key: string,
  targetKeyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): ReferenceFindResult {
  const params = handleParams(targetKeyValue.methodParams, mockBuffer, kvPath, rootKeyValue);
  let value: string;
  // 判断是否是函数
  if (typeof global[key].constructor === 'function') {
    value = key === 'Function' ? '() => {}' : `new ${key}(${params})`;
  } else {
    value = `${key}(${params})`;
  }
  return {
    keyValue: generateKeyValue(value, KeyValueTypes.VALUE),
    mockBuffer
  };
}

/**
 * 在当前文件中查找类型定义
 * @param key KV节点key值
 * @param targetKeyValue 需要查找的reference KV节点
 * @param parent 父节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findInCurrentFile(
  key: string,
  targetKeyValue: KeyValue,
  parent: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  property: KeyValue
): ReferenceFindResult {
  if (!parent) {
    return undefined as ReferenceFindResult;
  }
  if (parent.typeParameters[key] && parent.typeParameters[key] !== targetKeyValue) {
    return { keyValue: parent.typeParameters[key], mockBuffer } as ReferenceFindResult;
  }
  const foundKeyValue = parent.members[key];
  if (
    foundKeyValue &&
    foundKeyValue !== targetKeyValue &&
    (
      parent.type === KeyValueTypes.ENUM || foundKeyValue.type !== KeyValueTypes.PROPERTY
    )
  ) {
    if (foundKeyValue.type === KeyValueTypes.IMPORT) {
      return findDefFromImport(foundKeyValue, mockBuffer, rootKeyValue, property) as ReferenceFindResult;
    }
    const defKeyValue = findProperty(foundKeyValue, property);
    if (defKeyValue) {
      return {keyValue: defKeyValue, mockBuffer} as ReferenceFindResult;
    }
  }
  return findInCurrentFile(key, targetKeyValue, parent.parent, mockBuffer, kvPath, rootKeyValue, property) as ReferenceFindResult;
}

/**
 * 在全局声明中查找类型定义
 * @param key KV节点key值
 * @returns
 */
function findInDeclares(
  key: string
): ReferenceFindResult {
  if (DECLARES[key]) {
    const mockBuffer = mockBufferMap.get(MockedFileMap.get(DECLARES[key].from));
    return {
      keyValue: DECLARES[key].keyValue,
      mockBuffer,
      isGlobalDeclaration: path.basename(mockBuffer.rawFilePath).startsWith('@')
    } as ReferenceFindResult;
  } else {
    return undefined as ReferenceFindResult;
  }
}

/**
 * 在所有文件中查找类型定义
 * @param key KV节点key值
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findInAllFiles(
  key: string,
  property?: KeyValue
): ReferenceFindResult {
  for (const definedMockBuffer of mockBufferMap.values()) {
    const members = definedMockBuffer.contents.members;
    if (members[key]) {
      const defKeyValue = findProperty(members[key], property);
      return { keyValue: defKeyValue, mockBuffer: definedMockBuffer } as ReferenceFindResult;
    }
  }
  return undefined as ReferenceFindResult;
}

/**
 * 获取节点在当前文件的路径
 * 以递归的方式逐级向上获取所有祖先节点的key
 * @param keyValue KV节点
 * @param paths 节点路径
 * @returns
 */
function getKeyValuePath(keyValue: KeyValue, paths = []): string[] {
  if (!keyValue) {
    return paths;
  }
  paths.unshift(keyValue.key);
  return getKeyValuePath(keyValue.parent, paths);
}

/**
 * 处理同名函数
 * @param key
 * @param sameFuncList 同名函数列表
 * @param mockBuffer 当前文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleSameFunctions(
  key: string,
  sameFuncList: KeyValue[],
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const functionName = sameFuncList[0].key;
  if (sameFuncList.length >= 2) {
    return handleOverloadedFunction(key, sameFuncList, mockBuffer, kvPath, rootKeyValue, functionName);
  } else {
    return handleSingleFunction(key, sameFuncList, mockBuffer, kvPath, rootKeyValue, functionName);
  }
}

/**
 * 处理重载函数
 * @param key
 * @param sameFuncList 同名函数列表
 * @param mockBuffer 当前文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param functionName
 * @returns
 */
function handleOverloadedFunction(
  key: string,
  sameFuncList: KeyValue[],
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  functionName: string
): string {
  const func = sameFuncList.find(func => func.members.Promise);
  if (!func) {
    return handleSingleFunction(key, sameFuncList, mockBuffer, kvPath, rootKeyValue, functionName);
  }
  const promiseTypes = func.members.Promise;
  const memberLines: string[] = [];
  const returnData: string[] = [];
  const paramIndex: number = 1;
  Object.keys(promiseTypes.typeParameters).forEach(memberKey => {
    const memberKeyValue = promiseTypes.typeParameters[memberKey];
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberLines.push(`const p${paramIndex} = ${value}`);
    returnData.push(`p${paramIndex}`);
  });
  const isSpecial = specialOverloadedFunctionArr.includes(functionName);
  const paramMockData = handleFunParamMockData(sameFuncList, mockBuffer, kvPath, rootKeyValue, isSpecial, 'multiple', returnData);
  const isGetAccessor = key.startsWith('get ' + functionName);
  const isSetAccessor = key.startsWith('set ' + functionName);
  return `function (${isGetAccessor ? '' : `${isSetAccessor ? 'args' : '...args'}`}) {
    ${isGetAccessor || isSetAccessor ? '' : `console.warn(ts.replace('{{}}', '${func.key}'));`}
    ${memberLines.join(';\n')}${paramMockData ? '\n' + paramMockData : ''} 
    return new Promise(function (resolve, reject) {
      resolve(${returnData.join(', ')});
    });
  }`;
}

function handleFunParamMockData(
  funcList: KeyValue[],
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  isSpecial: boolean,
  funType: OverloadedFunctionType,
  returnData?: string[]
): string {
  let paramMockData = '';
  for (let i = 0; i < funcList.length; i++) {
    const funInfo = funcList[i];
    const { paramName, callBackParams, isAsyncCallback } = getCallbackMockData(funInfo, mockBuffer, kvPath, rootKeyValue, isSpecial);
    if (!callBackParams.length) {
      continue;
    }
    const returnInfo = isSpecial
      ? callBackParams.join(', ')
      : funType === 'single'
        ? callBackParams.join(', ')
        : returnData?.join(', ');
    const data = `if (args && typeof args[args.length - 1] === 'function') {
      args[args.length - 1].call(this, ${isAsyncCallback ? callbackError : ''}${returnInfo}); 
    }`;
    const info = (paramName ? `if(args && ['${paramName}'].includes(args[0])){\n` : '') + data + (paramName ? '}\n' : '');
    if (funType === 'single' || isSpecial || !isSpecial && !paramMockData) {
      paramMockData += info;
    }
  }
  return paramMockData;
}

function getCallbackMockData(
  funInfo: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  isSpecial: boolean
): CallbackParamMockData {
  let paramName = '';
  let isAsyncCallback = true;
  let callBackParams: string[] = [];
  Object.keys(funInfo.methodParams).forEach(key => {
    const paramInfo = funInfo.methodParams[key];
    const callbackData = handleCallbackParamMockData(key, paramInfo, callBackParams, paramName, isAsyncCallback, mockBuffer, kvPath, rootKeyValue, isSpecial);
    paramName = callbackData.paramName;
    isAsyncCallback = callbackData.isAsyncCallback;
    callBackParams = callbackData.callBackParams;
  });
  return {
    paramName,
    isAsyncCallback,
    callBackParams
  };
}

function handleCallbackParamMockData(
  key: string,
  paramInfo: KeyValue,
  callBackParams: string[],
  paramName: string,
  isAsyncCallback: boolean,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  isSpecial: boolean
): CallbackParamMockData {
  if (key === 'callback') {
    let callbackInfo: KeyValue;
    if (paramInfo.members.Callback) {
      isAsyncCallback = false;
      callbackInfo = paramInfo.members.Callback;
    }
    if (paramInfo.members.AsyncCallback) {
      isAsyncCallback = true;
      callbackInfo = paramInfo.members.AsyncCallback;
    }
    callbackInfo && Object.keys(callbackInfo.typeParameters).forEach(memberKey => {
      const memberKeyValue = callbackInfo.typeParameters[memberKey];
      const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
      callBackParams.push(value);
    });
  }
  if (key === 'type' && isSpecial) {
    Object.keys(paramInfo.members).forEach(memberKey => {
      if (!paramName) {
        paramName = memberKey;
      }
    });
  }

  return {
    isAsyncCallback,
    callBackParams,
    paramName
  };
}

/**
 * 拼接property
 * 通过递归方式，逐层将调用的属性拼接起来
 * 如：将{A:{key:A, property: {key:b, property: {key: c}}}}
 * 拼接成：A.b.c
 * @param property 调用的属性
 * @returns
 */
function concatProperty(property?: KeyValue): string {
  if (!property) {
    return '';
  }
  return `.${property.key}${concatProperty(property.property)}`;
}

/**
 * 获取获取定义KV节点的最后一个property
 *
 * @param keyValue 定义的KV节点
 * @param property 调用KV节点的属性
 * @returns
 */
function findProperty(keyValue: KeyValue, property?: KeyValue): KeyValue {
  const keyValueKey = keyValue.key;
  while (property && keyValue) {
    keyValue = keyValue.members[property.key];
    property = property.property;
  }
  if (!keyValue && property) {
    throw new Error(`未能在${keyValueKey}下找到${property.key}子孙属性`);
  }
  return keyValue;
}

/**
 * 处理property KV节点
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handlePropertyKeyValue(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const memberLines: string[] = [];
  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberLines.push(value);
  });
  return memberLines.join(',');
}

/**
 * 处理reference KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleReferenceKeyValue(
  key: string,
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  property: KeyValue
): string {
  if (IGNORE_REFERENCES.has(key)) {
    const memberLines: string[] = [];
    Object.keys(keyValue.typeParameters).forEach(memberKey => {
      const memberKeyValue = keyValue.typeParameters[memberKey];
      const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
      memberLines.push(value);
    });
    return memberLines.join(',\n');
  }
  const keyValueInfo = findKeyValueDefined(key, keyValue, mockBuffer, kvPath, rootKeyValue, keyValue.property);
  let value: string;

  if (keyValueInfo.isGlobalDeclaration) {
    const properties = concatProperty(keyValue.property);
    value = `global.${keyValueInfo.keyValue.key}${properties}`;
    const dependKeyValue = property ? keyValueInfo.keyValue.members[property.key] : keyValueInfo.keyValue;
    !dependKeyValue.isMocked && rootKeyValue.dependOnGlobals.add(dependKeyValue);
  } else {
    value = handleKeyValue(keyValueInfo.keyValue.key, keyValueInfo.keyValue, keyValueInfo.mockBuffer, kvPath, rootKeyValue, property);
  }

  if (value !== 'this') {
    switch (keyValueInfo.keyValue.type) {
      case KeyValueTypes.CLASS: {
        value = value.includes('global.uiMaterial') ? value : `new (${value})()`;
        break;
      }
      case KeyValueTypes.ENUM: {
        if (keyValue.parent.type !== KeyValueTypes.VARIABLE) {
          const firstMemberKey = Object.keys(keyValueInfo.keyValue.members)[0];
          const firstMemberKeyValue = keyValueInfo.keyValue.members[firstMemberKey];
          value = handleKeyValue(firstMemberKey, firstMemberKeyValue, keyValueInfo.mockBuffer, kvPath, rootKeyValue, firstMemberKeyValue.property);
        }
        break;
      }
    }
  }

  return value;
}

/**
 * 处理enum KV节点
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleEnumKeyValue(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const memberLines: string[] = ['isAutoMock: 0'];
  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberLines.push(`${memberKey}: ${value}`);
  });
  return `{${memberLines.join(',\n')}}`;
}

/**
 * 处理expression KV节点
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleExpressionKeyValue(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue
): string {
  const elements = keyValue.operateElements;
  return elements.map(element => {
    return handleKeyValue(element.key, element, mockBuffer, kvPath, rootKeyValue, element.property);
  }).join(' ');
}

/**
 * 处理非重载函数
 * @param key
 * @param sameFuncList
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param functionName
 * @returns
 */
function handleSingleFunction(
  key: string,
  sameFuncList: KeyValue[],
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  rootKeyValue: KeyValue,
  functionName: string
): string {
  const funcKeyValue = sameFuncList[0];
  const memberLines: string[] = [];
  Object.keys(funcKeyValue.members).forEach(memberKey => {
    const memberKeyValue = funcKeyValue.members[memberKey];
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberLines.push(value);
  });
  const isSpecial = specialOverloadedFunctionArr.includes(functionName);
  const funcList = isSpecial ? sameFuncList : [sameFuncList[0]];
  const paramMockData = handleFunParamMockData(funcList, mockBuffer, kvPath, rootKeyValue, isSpecial, 'single');

  const isGetAccessor = key.startsWith('get ' + functionName);
  const isSetAccessor = key.startsWith('set ' + functionName);
  const returnStr = funcKeyValue.members.Promise && funcKeyValue.members.Promise.type === KeyValueTypes.REFERENCE ?
    `return new Promise(function (resolve, reject) {
          resolve(${memberLines.join(',')});
        })` :
    `return ${memberLines.join(',')}`;

  return `function (${isGetAccessor ? '' : `${isSetAccessor ? 'args' : '...args'}`}) {
  ${isGetAccessor || isSetAccessor ? '' : `console.warn(ts.replace('{{}}', '${funcKeyValue.key}'));`}
  ${paramMockData ?? ''}${returnStr}
  }`;
}

/**
 * 获取KV节点的最后一个property的类型
 * @param keyValue KV节点
 * @param property 调用的属性
 * @returns
 */
function getKeyValueType(keyValue: KeyValue, property?: KeyValue): KeyValueTypes {
  while (property) {
    keyValue = keyValue.members[property.key];
    property = property.property;
  }
  return keyValue.type;
}

/**
 * 处理所有全局声明的KV节点
 * @param outMockJsFileDir mock文件输出路径
 */
export function handleDeclares(outMockJsFileDir: string): void {
  const declarations: string[] = [];
  const mockedDeclarations: Set<string> = new Set();
  Object.keys(DECLARES).forEach(key => {
    const keyValue = DECLARES[key].keyValue;
    switch (keyValue.type) {
      case KeyValueTypes.CLASS: {
        if (windowDeclaration.includes(key)) {
          declarations.push(`global.${key}_temp = class {constructor(){this.isAutoMock=true}};`);
        } else {
          declarations.push(`global.${key}_temp = class {constructor(){this.isAutoMock=true}};\nglobal.${key} = global.${key} || global.${key}_temp;`);
        }
        break;
      }
      case KeyValueTypes.INTERFACE:
      case KeyValueTypes.MODULE: {
        if (windowDeclaration.includes(key)) {
          declarations.push(`global.${key}_temp = {isAutoMock: true};`);
        } else {
          declarations.push(`global.${key}_temp = {isAutoMock: true};\nglobal.${key} = global.${key} || global.${key}_temp;`);
        }
      }
    }
  });

  Object.keys(DECLARES).forEach(key => {
    handleDeclare(DECLARES[key], declarations, mockedDeclarations);
  });

  const INTERVAL = 100;
  for (let counter = 0; counter < declarations.length; counter += INTERVAL) {
    const index = Math.floor(counter / INTERVAL) + 1;
    const filePath = path.join(outMockJsFileDir, `globalDeclarations${index}.js`);
    importDeclarationFiles.push(`import * as globalDeclarations${index} from './globalDeclarations${index}';`);
    const content = declarations.slice(counter, counter + INTERVAL).join('\n');
    fs.writeFileSync(filePath, content);
  }
}

/**
 * 处理全局声明的KV节点
 * @param declaration 全局声明的KV节点
 * @param declarations 所有全局声明的KV节点
 * @param mockedDeclarations 已mock的全局声明的KV节点的集合，避免重复mock
 * @param member 不为undefined时，只mock这个member节点
 * @returns
 */
function handleDeclare(
  declaration: Declare,
  declarations: string[],
  mockedDeclarations: Set<string>,
  member?: KeyValue
): void {
  if (member?.isMocked) {
    return;
  }
  const keyValue = declaration.keyValue;
  const key = keyValue.key;
  const mockBuffer = mockBufferMap.get(MockedFileMap.get(declaration.from));

  const values: string[] = [];
  switch (keyValue.type) {
    case KeyValueTypes.FUNCTION: {
      if (!mockedDeclarations.has(key)) {
        const functionBody = handleKeyValue(key, keyValue, mockBuffer, [], keyValue, keyValue.property);
        const value = `global.${key} = global.${key} || (${functionBody});`;
        values.push(value);
        mockedDeclarations.add(key);
      }
      break;
    }
    case KeyValueTypes.CLASS: {
      handleGlobalClass(keyValue, mockBuffer, values, [keyValue], member);
      break;
    }
    case KeyValueTypes.MODULE: {
      handleGlobalModule(keyValue, mockBuffer, values, [keyValue], member);
      break;
    }
    case KeyValueTypes.INTERFACE: {
      handleGlobalInterface(keyValue, mockBuffer, values, [keyValue], member);
      break;
    }
    default: {
      if (!mockedDeclarations.has(key)) {
        const value = `global.${key} = global.${key} || (${handleKeyValue(key, keyValue, mockBuffer, [], keyValue, keyValue.property)});`;
        values.push(value);
        mockedDeclarations.add(key);
      }
      break;
    }
  }
  handleDependOnGlobals(keyValue, declarations, mockedDeclarations);
  Array.prototype.push.apply(declarations, values);
}

/**
 * 处理KV节点用到的全局节点
 * @param keyValue KV节点
 * @param declarations 已mock的文本内容
 * @param mockedDeclarations 已mock的全局节点的集合
 * @returns
 */
function handleDependOnGlobals(
  keyValue: KeyValue,
  declarations: string[],
  mockedDeclarations: Set<string>
): void {
  if (keyValue.type === KeyValueTypes.FUNCTION) {
    return;
  }
  keyValue.dependOnGlobals.forEach(dependKeyValue => {
    const isMockedDeclarationsHas = isMockedDeclarations(DECLARES[dependKeyValue.key], mockedDeclarations);
    if (dependKeyValue.isGlobalDeclare && !isMockedDeclarationsHas) {
      handleDeclare(DECLARES[dependKeyValue.key], declarations, mockedDeclarations);
    } else if (dependKeyValue.parent.isGlobalDeclare) {
      handleDeclare(DECLARES[dependKeyValue.parent.key], declarations, mockedDeclarations, dependKeyValue);
    } else if (isMockedDeclarationsHas) {
      return;
    } else {
      throw new Error(`${keyValue.key}非全局节点。`);
    }
  });
}

/**
 * 处理全局class KV节点
 * @param keyValue class类型的KV节点
 * @param mockBuffer mock信息
 * @param declarations 已mock的文本内容
 * @param kvPath KV节点路径
 * @param member 不为undefined时，只mock这个member节点
 * @returns
 */
function handleGlobalClass(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  declarations: string[],
  kvPath: KeyValue[],
  member?: KeyValue
): void {
  if (member) {
    if (!member.isMocked) {
      const memberValue = handleKeyValue(member.key, member, mockBuffer, kvPath, keyValue, member.property);
      const functionKeyword = member.type === KeyValueTypes.FUNCTION ? 'function' : '';
      const prototypeStr = member.isStatic ? '' : '.prototype';
      const value = `global.${keyValue.key}_temp${prototypeStr}.${member.key} = ${functionKeyword}${memberValue};`;
      member.isMocked = true;
      declarations.push(value);
    }
    return;
  }
  if (keyValue.heritage) {
    handleHeritage(keyValue, mockBuffer, kvPath.concat(keyValue), keyValue);
  }

  Object.keys(keyValue.members).forEach(
    memberKey => handleClassMembers(memberKey, keyValue, mockBuffer, kvPath, declarations)
  );
  // 处理同名declare
  keyValue.sameDeclares.forEach(sameDeclare => {
    const sameKeyValue = sameDeclare.keyValue;
    const sameMockBuffer = mockBufferMap.get(MockedFileMap.get(sameDeclare.from));
    Object.keys(sameKeyValue.members).forEach(
      memberKey => handleClassMembers(memberKey, sameKeyValue, sameMockBuffer, kvPath, declarations)
    );
  });
}

/**
 * 处理class KV节点的属性和方法
 * @param memberKey 属性或方法的名称
 * @param parent 父级class KV节点
 * @param mockBuffer 所属文件的mock信息
 * @param kvPath KV节点路径
 * @param declarations 已mock的文本内容
 * @returns
 */
function handleClassMembers(
  memberKey: string,
  parent: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  declarations: string[]
): void {
  const memberKeyValue = parent.members[memberKey];
  if (memberKeyValue.isMocked) {
    return;
  }
  let elementName = `.${memberKey}`;
  if (memberKeyValue.type === KeyValueTypes.EXPRESSION) {
    memberKeyValue.key = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, parent, memberKeyValue.property);
    memberKeyValue.type = KeyValueTypes.FUNCTION;
    memberKeyValue.value = undefined;
    elementName = memberKeyValue.key;
  }

  const memberValue = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, parent, memberKeyValue.property);
  let value: string;
  if (memberKeyValue.type === KeyValueTypes.FUNCTION) {
    value = handleClassMethod(memberKey, memberKeyValue, parent, mockBuffer, kvPath, elementName, memberValue);
  } else {
    value = `global.${parent.key}_temp${memberKeyValue.isStatic ? '' : '.prototype'}${elementName} = ${memberValue};`;
  }
  if (!memberKeyValue.isStatic && memberKeyValue.sameName.some(sameKeyValue => sameKeyValue.isStatic)) {
    value += `\nglobal.${parent.key}_temp${elementName} = global.${parent.key}_temp.prototype${elementName}`;
  }
  memberKeyValue.isMocked = true;
  declarations.push(value);
}

/**
 * 处理全局module KV节点
 * @param keyValue KV节点
 * @param mockBuffer 所属文件的mock信息
 * @param declarations 已mock的文本内容
 * @param kvPath KV节点路径
 * @param member 不为undefined时，只mock这个member节点
 * @returns
 */
function handleGlobalModule(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  declarations: string[],
  kvPath: KeyValue[],
  member?: KeyValue
): void {
  return handleGlobalModuleOrInterface(keyValue, mockBuffer, declarations, kvPath, member);
}

/**
 * 处理全局module或interface KV节点
 * @param keyValue KV节点
 * @param mockBuffer 所属文件的mock信息
 * @param declarations 已mock的文本内容
 * @param kvPath KV节点路径
 * @param member 不为undefined时，只mock这个member节点
 * @returns
 */
function handleGlobalModuleOrInterface(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  declarations: string[],
  kvPath: KeyValue[],
  member?: KeyValue
): void {
  if (member) {
    if (!member.isMocked) {
      const memberKey = member.key;
      const memberValue = handleKeyValue(memberKey, member, mockBuffer, kvPath, keyValue, member.property);
      const value = `global.${keyValue.key}_temp.${memberKey} = ${memberValue};`;
      member.isMocked = true;
      declarations.push(value);
    }
    return;
  }
  Object.keys(keyValue.members).forEach(memberKey => handleModuleOrInterfaceMember(memberKey, keyValue, mockBuffer, kvPath, declarations, keyValue));
  // 处理同名declare
  keyValue.sameDeclares.forEach(sameDeclare => {
    const sameKeyValue = sameDeclare.keyValue;
    const sameMockBuffer = mockBufferMap.get(MockedFileMap.get(sameDeclare.from));
    const needHandleTypes = new Set([KeyValueTypes.CLASS, KeyValueTypes.INTERFACE, KeyValueTypes.MODULE]);
    if (!needHandleTypes.has(sameKeyValue.type) && path.basename(sameMockBuffer.rawFilePath).startsWith('@')) {
      return;
    }
    Object.keys(sameKeyValue.members).forEach(
      memberKey => handleModuleOrInterfaceMember(memberKey, sameKeyValue, sameMockBuffer, kvPath, declarations, keyValue)
    );
  });
}

/**
 * 处理module和interface的成员
 * @param memberKey 成员名称
 * @param parent 成员父节点
 * @param mockBuffer 当前文件mock信息
 * @param kvPath KV节点路径
 * @param declarations 已mock的全局接口
 * @param rootKeyValue
 */
function handleModuleOrInterfaceMember(
  memberKey: string,
  parent: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  declarations: string[],
  rootKeyValue: KeyValue
): void {
  const memberKeyValue = parent.members[memberKey];
  if (memberKeyValue.isMocked) {
    return;
  }
  let elementName = `.${memberKey}`;
  if (memberKeyValue.type === KeyValueTypes.EXPRESSION) {
    memberKeyValue.key = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberKeyValue.type = KeyValueTypes.PROPERTY;
    memberKeyValue.value = undefined;
    elementName = memberKeyValue.key;
  }
  const memberValue = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
  const value = `global.${parent.key}_temp${elementName} = ${memberValue};`;
  memberKeyValue.isMocked = true;
  declarations.push(value);
}

/**
 * 处理全局interface KV节点
 * @param keyValue KV节点
 * @param mockBuffer 所属文件的mock信息
 * @param declarations 已mock的文本内容
 * @param kvPath KV节点路径
 * @param member 不为undefined时，只mock这个member节点
 * @returns
 */
function handleGlobalInterface(
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  declarations: string[],
  kvPath: KeyValue[],
  member?: KeyValue
): void {
  if (keyValue.heritage) {
    handleHeritage(keyValue, mockBuffer, kvPath.concat(keyValue), keyValue);
  }
  return handleGlobalModuleOrInterface(keyValue, mockBuffer, declarations, kvPath, member);
}

/**
 * 从导入节点向上查找类型定义
 * @param importKeyValue 导入的KV节点
 * @param mockBuffer 当前文件的mock信息
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findDefFromImport(
  importKeyValue: KeyValue,
  mockBuffer: MockBuffer,
  rootKeyValue: KeyValue,
  property?: KeyValue
): ReferenceFindResult {
  const importedMockBuffer = mockBufferMap.get(MockedFileMap.get(importKeyValue.importedModulePath));
  if (!importedMockBuffer) {
    throw new Error('未找到foundKeyValue.importedModulePath对应的mockBuffer');
  }
  let defKeyValue: KeyValue;
  if (importKeyValue.isImportDefault) {
    defKeyValue = importedMockBuffer.contents.members.default;
  } else if (importKeyValue.isNamespaceImport) {
    defKeyValue = importedMockBuffer.contents;
  } else {
    defKeyValue = importedMockBuffer.contents.members[importKeyValue.rawName ?? importKeyValue.key];
  }
  if (defKeyValue.isGlobalDeclare) {
    const dependKeyValue = property ? defKeyValue.members[property.key] : defKeyValue;
    if (dependKeyValue.type === KeyValueTypes.ENUM) {
      defKeyValue = dependKeyValue;
    } else {
      !dependKeyValue.isMocked && rootKeyValue.dependOnGlobals.add(dependKeyValue);
      const keyValueType = getKeyValueType(defKeyValue, property);
      const newKey = `global.${defKeyValue.key}${concatProperty(property)}`;
      defKeyValue = generateKeyValue(newKey, keyValueType);
      defKeyValue.value = newKey;
      !dependKeyValue.isMocked && defKeyValue.dependOnGlobals.add(dependKeyValue);
    }
  } else {
    defKeyValue = findProperty(defKeyValue, property);
  }

  if (!defKeyValue) {
    const value = `Not exported ${importKeyValue.rawName ?? importKeyValue.key} from ${importedMockBuffer.rawFilePath} in ${mockBuffer.rawFilePath}`.replace(/\\/g, '/');
    console.error(value);
    defKeyValue = generateKeyValue(value, KeyValueTypes.VALUE, importedMockBuffer.contents);
    defKeyValue.value = `'${value}'`;
  }
  return { keyValue: defKeyValue, mockBuffer: importedMockBuffer };
}

/**
 * mock类方法
 * @param memberKey 方法原名
 * @param memberKeyValue 方法KV节点
 * @param parent  负极节点
 * @param mockBuffer 当前文件的mock信息
 * @param kvPath KV检点路径
 * @param elementName 方法名转换后方法名
 * @param memberValue 类方法的mock内容
 */
function handleClassMethod(
  memberKey: string,
  memberKeyValue: KeyValue,
  parent: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[],
  elementName: string,
  memberValue: string
): string {
  let value:string;
  if (memberKey.startsWith('get ') || memberKey.startsWith('set ')) {
    value = handleClassGetterOrSetterMethod(memberKeyValue, parent, mockBuffer, kvPath);
  } else {
    value = `global.${parent.key}_temp${memberKeyValue.isStatic ? '' : '.prototype'}${elementName} = ${memberValue};`;
  }
  return value;
}

/**
 * mock 类中带get和set关键字的方法
 * @param memberKeyValue 类方法KV节点
 * @param parent 父级KV节点
 * @param mockBuffer 当前文件的mock信息
 * @param kvPath KV节点路径
 */
function handleClassGetterOrSetterMethod(
  memberKeyValue: KeyValue,
  parent: KeyValue,
  mockBuffer: MockBuffer,
  kvPath: KeyValue[]
): string {
  const getKey = `get ${memberKeyValue.key}`;
  let getMethodValue: string = '';
  if (parent.members[getKey]) {
    const getFunctionBody = handleKeyValue(getKey, parent.members[getKey], mockBuffer, kvPath, parent, memberKeyValue.property);
    getMethodValue = `get: ${getFunctionBody},`;
  }

  let setMethodValue: string = '';
  const setKey = `set ${memberKeyValue.key}`;
  if (parent.members[setKey]) {
    const setFunctionBody = handleKeyValue(setKey, parent.members[setKey], mockBuffer, kvPath, parent, memberKeyValue.property);
    setMethodValue = `set: ${setFunctionBody},`;
  }

  if (parent.members[getKey]) {
    parent.members[getKey].isMocked = true;
  }
  if (parent.members[setKey]) {
    parent.members[setKey].isMocked = true;
  }

  return `Object.defineProperty(global.${parent.key}_temp, '${memberKeyValue.key}', {
  ${getMethodValue}
  ${setMethodValue}
});`;
}
