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
import { DECLARES, IGNORE_REFERENCES, importDeclarationFiles, KeyValueTypes, mockBufferMap, MockedFileMap, TSTypes, TYPESCRIPT_KEYWORDS } from '../common/constants';
import { Declare, KeyValue, Members, MockBuffer } from '../types';
import { generateKeyValue } from '../common/commonUtils';

interface KeyValueInfo {
  keyValue: KeyValue,
  mockBuffer: MockBuffer,
  isGlobalDeclaration?: boolean
}

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
    if (keyValue.isGlobalDeclare) {
      membersContent.push(`export const ${memberKey}=global.${memberKey};`);
    } else {
      if (keyValue.type === KeyValueTypes.FUNCTION) {
        membersContent.push(`export function ${memberKey}${handleKeyValue(memberKey, keyValue, mockBuffer, [], keyValue, keyValue.property)};`);
      } else {
        membersContent.push(`export const ${memberKey} = ${handleKeyValue(memberKey, keyValue, mockBuffer, [], keyValue, keyValue.property)};`);
      }
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
function handleKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
  if (keyValue.value !== undefined) {
    return keyValue.value;
  }
  if (new Set<KeyValueTypes>([KeyValueTypes.CLASS, KeyValueTypes.MODULE, KeyValueTypes.INTERFACE]).has(keyValue.type) && kvPath.includes(keyValue)) {
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
function mockKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
  let value: string;
  switch (keyValue.type) {
    case KeyValueTypes.CLASS: {
      value = handleClassKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.EXPORT: {
      value = handleExportKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.FILE: {
      value = handleFileKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.FUNCTION: {
      value = handleFunctionKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.IMPORT: {
      value = handleImportKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.INTERSECTION: {
      value = handleIntersectionKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.MODULE: {
      value = handleModuleKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.INTERFACE: {
      value = handleInterfaceKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.VALUE: {
      value = handleValueKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.VARIABLE: {
      value = handleVariableKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.PROPERTY: {
      value = handlePropertyKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.REFERENCE: {
      value = handleReferenceKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.ENUM: {
      value = handleEnumKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
    case KeyValueTypes.EXPRESSION: {
      value = handleExpressionKeyValue(key, keyValue, mockBuffer, kvPath, rootKeyValue, property);
      break;
    }
  }
  keyValue.value = value;
  return value;
}

/**
 * 处理class KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleClassKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
  const memberLines: string[] = [];
  const dynamicProperties: string[] = [];

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
      if (memberKeyValue.members['IterableIterator']) {
        memberLines.push(`*${elementName}${value}`);
      } else {
        memberLines.push(`${memberKeyValue.isStatic ? 'static ' : ''}${elementName}${value}`);
      }
    } else {
      if (memberKeyValue.isStatic) {
        memberLines.push(`static ${elementName} = ${value}`);
      } else {
        dynamicProperties.push(`this.${elementName} = ${value}`);
      }
    }
  });

  return `class {constructor() {\n${dynamicProperties.join(';\n')}\n}\n${memberLines.join(';\n')}\n}`;
}

/**
 * 处理继承
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleHeritage(keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue) {
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
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleExportKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
function handleFileKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
 * 处理funciton KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleFunctionKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
  return handleSameFunctions(sameFuncList, mockBuffer, kvPath, rootKeyValue);
}

/**
 * 处理import KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleImportKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleIntersectionKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
function handleModuleKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property?: KeyValue): string {
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
    let value: string;
    if (memberKeyValue.type === KeyValueTypes.FUNCTION) {
      value = `${memberKeyValue.key}${handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property)}`;
    } else {
      value = `${memberKeyValue.key}: ${handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property)}`;
    }
    memberLines.push(value);
  });
  return `{\n${memberLines.join(',\n')}\n}`;
}

/**
 * 处理interface KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleInterfaceKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
  const memberLines: string[] = [];

  if (keyValue.heritage) {
    handleHeritage(keyValue, mockBuffer, kvPath.concat([keyValue.heritage]), rootKeyValue);
  }
  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    let value: string;
    if (memberKeyValue.type === KeyValueTypes.FUNCTION) {
      value = `${memberKeyValue.key}${handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property)}`;
    } else {
      value = `${memberKeyValue.key}: ${handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property)}`;
    }
    memberLines.push(value);
  });
  return `{\n${memberLines.join(',\n')}\n}`;
}

/**
 * 处理value KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleValueKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
  return keyValue.key;
}

/**
 * 处理variable KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleVariableKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
function findKeyValueDefined(key: string, targetKeyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): KeyValueInfo {
  let keyValueInfo: KeyValueInfo | undefined;
  // 在当前文件中找
  keyValueInfo = findInCurrentFile(key, targetKeyValue, targetKeyValue.parent, mockBuffer, kvPath, rootKeyValue, property);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 在全局定义中找
  keyValueInfo = findInDeclares(key, targetKeyValue, kvPath, property);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 在js库中找
  keyValueInfo = findInLibs(key, targetKeyValue, mockBuffer, kvPath, rootKeyValue, property);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 在TS内置类型中找
  keyValueInfo = findTSTypes(key, targetKeyValue, mockBuffer, kvPath, rootKeyValue, property);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  // 在所有文件中找
  keyValueInfo = findInAllFiles(key, targetKeyValue, kvPath, property);
  if (keyValueInfo) {
    return keyValueInfo;
  }

  const keyValuePath: string[] = getKeyValuePath(targetKeyValue);
  const value: string = `Cannot find type definition for ${keyValuePath.slice(1).join('->')} from file ${MockedFileMap.get(keyValuePath[1])}`;
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
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findTSTypes(key: string, targetKeyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): KeyValueInfo | undefined {
  const paramsContent: string[] = [];
  Object.keys(targetKeyValue.typeParameters).forEach(typeParameter => {
    const typeParamterKeyValue: KeyValue = targetKeyValue.typeParameters[typeParameter];
    const paramContent: string = handleKeyValue(typeParameter, typeParamterKeyValue, mockBuffer, kvPath, rootKeyValue, typeParamterKeyValue.property);
    paramsContent.push(paramContent);
  });
  if (!TSTypes[key]) {
    return;
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
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleParams(params: Members, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findInLibs(key: string, targetKeyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): KeyValueInfo | undefined {
  if (key === 'globalThis') {
    const globalThisKeyValue = generateKeyValue(key, KeyValueTypes.VALUE);
    return { keyValue: globalThisKeyValue, mockBuffer };
  }
  if (!global[key]) {
    return;
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
      return findInLibFunction(key, targetKeyValue, mockBuffer, kvPath, rootKeyValue, property)
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
}

/**
 * 在库函数中查找类型定义
 * @param key KV节点key值
 * @param targetKeyValue 需要查找的reference KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findInLibFunction(key: string, targetKeyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): KeyValueInfo | undefined {
  const params = handleParams(targetKeyValue.methodParams, mockBuffer, kvPath, rootKeyValue, property);
  let value: string;
  // 判断是否是函数
  if (typeof global[key].constructor === 'function') {
    value = `new ${key}(${params})`;
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
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findInCurrentFile(key: string, targetKeyValue: KeyValue, parent: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): KeyValueInfo | undefined {
  if (!parent) {
    return;
  }
  if (parent.typeParameters[key] && parent.typeParameters[key] !== targetKeyValue) {
    return { keyValue: parent.typeParameters[key], mockBuffer };
  }
  const foundKeyValue = parent.members[key];
  if (foundKeyValue && foundKeyValue !== targetKeyValue) {
    if (foundKeyValue.type === KeyValueTypes.IMPORT) {
      return findDefFromImport(foundKeyValue, mockBuffer, rootKeyValue, property);
    }
    const defKeyValue = findProperty(foundKeyValue, property);
    if (defKeyValue) {
      return {keyValue: defKeyValue, mockBuffer};
    }
  }
  return findInCurrentFile(key, targetKeyValue, parent.parent, mockBuffer, kvPath, rootKeyValue, property);
}

/**
 * 在全局声明中查找类型定义
 * @param key KV节点key值
 * @param targetKeyValue 需要查找的reference KV节点
 * @param kvPath KV节点路径
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findInDeclares(key: string, targetKeyValue: KeyValue, kvPath: KeyValue[], property?: KeyValue): KeyValueInfo | undefined {
  if (DECLARES[key]) {
    const mockBuffer = mockBufferMap.get(MockedFileMap.get(DECLARES[key].from));
    return {
      keyValue: DECLARES[key].keyValue,
      mockBuffer,
      isGlobalDeclaration: path.basename(mockBuffer.rawFilePath).startsWith('@')
    };
  } else {
    return;
  }
}

/**
 * 在所有文件中查找类型定义
 * @param key KV节点key值
 * @param targetKeyValue 需要查找的reference KV节点
 * @param kvPath KV节点路径
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function findInAllFiles(key: string, targetKeyValue: KeyValue, kvPath: KeyValue[], property?: KeyValue): KeyValueInfo | undefined {
  for (const definedMockBuffer of mockBufferMap.values()) {
    const members = definedMockBuffer.contents.members;
    if (members[key]) {
      const defKeyValue = findProperty(members[key], property);
      return { keyValue: defKeyValue, mockBuffer: definedMockBuffer };
    }
  }
}

/**
 * 获取节点在当前文件的路径
 * 以递归的方式逐级向上获取所有祖先节点的key
 * @param keyValue KV节点
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
 * @param sameFuncList 同名函数列表
 * @param mockBuffer 当前文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleSameFunctions(sameFuncList: KeyValue[], mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue): string {
  if (sameFuncList.length >= 2) {
    return handleOverloadedFunction(sameFuncList, mockBuffer, kvPath, rootKeyValue);
  } else {
    return handleSingleFunction(sameFuncList[0], mockBuffer, kvPath, rootKeyValue);
  }
}

/**
 * 处理重载函数
 * @param sameFuncList 同名函数列表
 * @param mockBuffer 当前文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @returns
 */
function handleOverloadedFunction(sameFuncList: KeyValue[], mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue): string {
  const func = sameFuncList.find(func => func.members.Promise);
  if (!func) {
    return handleSingleFunction(sameFuncList[0], mockBuffer, kvPath, rootKeyValue);
  }
  const promiseTypes = func.members.Promise;
  const memberLines: string[] = [];
  const callBackParams: string[] = [];
  const paramIndex: number = 1;
  Object.keys(promiseTypes.typeParameters).forEach(memberKey => {
    const memberKeyValue = promiseTypes.typeParameters[memberKey];
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberLines.push(`const p${paramIndex} = ${value}`);
    callBackParams.push(`p${paramIndex}`);
  });
  let isAsyncCallback = true;
  const callbackError = '{\'code\': \'\',\'data\': \'\',\'name\': \'\',\'message\': \'\',\'stack\': \'\'}, ';
  for (let i = 0; i < sameFuncList.length; i++) {
    const element = sameFuncList[i];
    Object.keys(element.methodParams).forEach(key => {
      const callbackInfo = element.methodParams[key];
      if (callbackInfo && Object.keys(callbackInfo.members).includes('Callback')) {
        isAsyncCallback = false;
      }
    });
    if (!isAsyncCallback) {
      break;
    }
  }
  return `(...args) {
    console.warn(ts.replace('{{}}', '${func.key}'));
    ${memberLines.join(';\n')}
    if (args && typeof args[args.length - 1] === 'function') {
      args[args.length - 1].call(this, ${isAsyncCallback ? callbackError : ''}${callBackParams.join(', ')});
    }
    return new Promise((resolve, reject) => {
      resolve(${callBackParams.join(', ')});
    });
  }`;
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
function findProperty(keyValue: KeyValue, property?: KeyValue): KeyValue | undefined {
  while (property && keyValue) {
    keyValue = keyValue.members[property.key];
    property = property.property;
  }
  if (!keyValue && property) {
    return;
  }
  return keyValue;
}

/**
 * 处理property KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handlePropertyKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
function handleReferenceKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
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
        value = `new (${value})()`;
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

  if (keyValueInfo.keyValue.type === KeyValueTypes.FUNCTION) {
    value = `function ${value}`;
  }
  return value;
}

/**
 * 处理enum KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleEnumKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
  const memberLines: string[] = [];
  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberLines.push(`${memberKey}: ${value}`);
  });
  return `{${memberLines.join(',\n')}}`;
}

/**
 * 处理expression KV节点
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleExpressionKeyValue(key: string, keyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue, property: KeyValue): string {
  const elements = keyValue.operateElements;
  return elements.map(element => {
    return handleKeyValue(element.key, element, mockBuffer, kvPath, rootKeyValue, element.property);
  }).join(' ');
}

/**
 * 处理非重载函数
 * @param key KV节点key值
 * @param keyValue KV节点
 * @param mockBuffer KV节点所在文件的mock信息
 * @param kvPath KV节点路径
 * @param rootKeyValue 仅次于FILE节点的根节点
 * @param property KV节点的调用属性节点，如A.b, b节点为property
 * @returns
 */
function handleSingleFunction(funcKeyValue: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], rootKeyValue: KeyValue): string {
  const memberLines: string[] = [];
  Object.keys(funcKeyValue.members).forEach(memberKey => {
    const memberKeyValue = funcKeyValue.members[memberKey];
    const value = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, rootKeyValue, memberKeyValue.property);
    memberLines.push(value);
  });
  const methodParams = Object.keys(funcKeyValue.methodParams).map(
    methodParam => TYPESCRIPT_KEYWORDS.has(methodParam) ? `${methodParam}1` : methodParam
  ).join(', ');
  const returnStr = funcKeyValue.members.Promise && funcKeyValue.members.Promise.type === KeyValueTypes.REFERENCE ?
    `return new Promise((resolve, reject) => {
          resolve(${memberLines.join(',')});
        })` :
    `return ${memberLines.join(',')}`;
  return `(${methodParams}) ${funcKeyValue.isArrowFunction ? '=>' : ''} {
  console.warn(ts.replace('{{}}', '${funcKeyValue.key}'));
  ${returnStr}
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
        declarations.push(`global.${key}_temp = class {};\nglobal.${key} = global.${key} || global.${key}_temp;`);
        break;
      }
      case KeyValueTypes.INTERFACE:
      case KeyValueTypes.MODULE: {
        declarations.push(`global.${key}_temp = {};\nglobal.${key} = global.${key} || global.${key}_temp;`);
      }
    }
  });

  Object.keys(DECLARES).forEach(key => {
    handleDeclare(DECLARES[key], declarations, mockedDeclarations);
  });

  const INTERVAL = 20;
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
function handleDeclare(declaration: Declare, declarations: string[], mockedDeclarations: Set<string>, member?: KeyValue): void {
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
        const value = `global.${key} = global.${key} || (function ${handleKeyValue(key, keyValue, mockBuffer, [], keyValue, keyValue.property)});`;
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
function handleDependOnGlobals(keyValue: KeyValue, declarations: string[], mockedDeclarations: Set<string>): void {
  if (keyValue.type === KeyValueTypes.FUNCTION) {
    return;
  }
  keyValue.dependOnGlobals.forEach(dependKeyValue => {
    if (dependKeyValue.isGlobalDeclare) {
      handleDeclare(DECLARES[dependKeyValue.key], declarations, mockedDeclarations);
    } else if (dependKeyValue.parent.isGlobalDeclare) {
      handleDeclare(DECLARES[dependKeyValue.parent.key], declarations, mockedDeclarations, dependKeyValue);
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
function handleGlobalClass(keyValue: KeyValue, mockBuffer: MockBuffer, declarations: string[], kvPath: KeyValue[], member?: KeyValue): void {
  if (member) {
    if (!member.isMocked) {
      const memberValue = handleKeyValue(member.key, member, mockBuffer, kvPath, keyValue, member.property);
      const value = `global.${keyValue.key}_temp${member.isStatic ? '' : '.prototype'}.${member.key} = ${member.type === KeyValueTypes.FUNCTION ? 'function' : ''}${memberValue};`;
      member.isMocked = true;
      declarations.push(value);
    }
    return;
  }
  if (keyValue.heritage) {
    handleHeritage(keyValue, mockBuffer, kvPath.concat(keyValue), keyValue);
  }

  Object.keys(keyValue.members).forEach(memberKey => handleClassMembers(memberKey, keyValue, mockBuffer, kvPath, declarations));
  // 处理同名declare
  keyValue.sameDeclares.forEach(sameDeclare => {
    const sameKeyValue = sameDeclare.keyValue;
    const sameMockBuffer =  mockBufferMap.get(MockedFileMap.get(sameDeclare.from));
    Object.keys(sameKeyValue.members).forEach(memberKey => handleClassMembers(memberKey, sameKeyValue, sameMockBuffer, kvPath, declarations));
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
function handleClassMembers(memberKey: string, parent: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], declarations: string[]): void {
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

  const star = memberKeyValue.type === KeyValueTypes.FUNCTION && memberKeyValue.members['IterableIterator'] ? '*' : '';
  const memberValue = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, parent, memberKeyValue.property);
  let value = '';
  if (memberKeyValue.type === KeyValueTypes.FUNCTION) {
    value = handleClassMethod(memberKey, memberKeyValue, star, parent, mockBuffer, kvPath, elementName, memberValue);
  } else {
    value = `global.${parent.key}_temp${memberKeyValue.isStatic ? '' : '.prototype'}${elementName} = ${memberValue};`;
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
function handleGlobalModule(keyValue: KeyValue, mockBuffer: MockBuffer, declarations: string[], kvPath: KeyValue[], member?: KeyValue): void {
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
function handleGlobalModuleOrInterface(keyValue: KeyValue, mockBuffer: MockBuffer, declarations: string[], kvPath: KeyValue[], member?: KeyValue): void {
  if (member) {
    if (!member.isMocked) {
      const memberKey = member.key;
      const memberValue = handleKeyValue(memberKey, member, mockBuffer, kvPath, keyValue, member.property);
      const value = `global.${keyValue.key}_temp.${memberKey} = ${member.type === KeyValueTypes.FUNCTION ? 'function' : ''}${memberValue};`;
      member.isMocked = true;
      declarations.push(value);
    }
    return;
  }
  Object.keys(keyValue.members).forEach(memberKey => {
    const memberKeyValue = keyValue.members[memberKey];
    if (memberKeyValue.isMocked) {
      return;
    }
    let elementName = `.${memberKey}`;
    if (memberKeyValue.type === KeyValueTypes.EXPRESSION) {
      memberKeyValue.key = handleKeyValue(memberKey, memberKeyValue, mockBuffer, kvPath, keyValue, memberKeyValue.property);
      memberKeyValue.type = KeyValueTypes.PROPERTY;
      memberKeyValue.value = undefined;
      elementName = memberKeyValue.key;
    }
    const memberValue = handleKeyValue(elementName, memberKeyValue, mockBuffer, kvPath, keyValue, memberKeyValue.property);
    const value = `global.${keyValue.key}_temp${elementName} = ${memberKeyValue.type === KeyValueTypes.FUNCTION ? 'function' : ''}${memberValue};`;
    memberKeyValue.isMocked = true;
    declarations.push(value);
  });
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
function handleGlobalInterface(keyValue: KeyValue, mockBuffer: MockBuffer, declarations: string[], kvPath: KeyValue[], member?: KeyValue): void {
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
function findDefFromImport(importKeyValue: KeyValue, mockBuffer: MockBuffer, rootKeyValue: KeyValue, property?: KeyValue): KeyValueInfo {
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

function handleClassMethod(memberKey: string, memberKeyValue: KeyValue, star: string, parent: KeyValue, mockBuffer: MockBuffer, kvPath: KeyValue[], elementName: string, memberValue: string): string {
  let value:string;
  if (memberKey.startsWith('get ') || memberKey.startsWith('set ')) {
    const getKey = `get ${memberKeyValue.key}`;
    const getMethodValue = parent.members[getKey] ? `get: function${star} ${handleKeyValue(getKey, parent.members[getKey], mockBuffer, kvPath, parent, memberKeyValue.property)},` : '';
    const setKey = `set ${memberKeyValue.key}`;
    const setMethodValue = parent.members[setKey] ? `set: function${star} ${handleKeyValue(setKey, parent.members[setKey], mockBuffer, kvPath, parent, memberKeyValue.property)},` : '';
    value = `Object.defineProperty(global.${parent.key}_temp, '${memberKeyValue.key}', {
  ${getMethodValue}
  ${setMethodValue}
});`;
    if (parent.members[getKey]) {
      parent.members[getKey].isMocked = true;
    }
    if (parent.members[setKey]) {
      parent.members[setKey].isMocked = true;
    }
  } else {
    value = `global.${parent.key}_temp${memberKeyValue.isStatic ? '' : '.prototype'}${elementName} = function${star} ${memberValue};`;
  }
  return value;
}
