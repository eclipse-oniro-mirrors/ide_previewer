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
import fs from 'fs';
import { Node, SourceFile, JSDoc } from 'typescript';
import { KeyValue, Members, MockBuffer } from '../types';
import { D_ETS, D_TS, KeyValueTypes, NAPI_DIR_PATH, needShieldingDirOrFile } from './constants';
import { getProjectDir } from './systemUtils';

/**
 * 递归创建文件夹
 * @param dirname 需要创建文件夹的路径
 * @returns 是否创建成功
 */
export function mkdirsSync(dirname: string): boolean {
  if (fs.existsSync(dirname)) {
    return true;
  } else {
    if (mkdirsSync(path.dirname(dirname))) {
      fs.mkdirSync(dirname);
      return true;
    }
  }
  return false;
}

/**
 * 生成KeyValue节点
 * @param key 节点名称
 * @param type 节点类型
 * @param parent 该节点的父节点
 * @param optionMembers 其他初始化参数
 * @returns KeyValue节点
 */
export function generateKeyValue(key: string, type: KeyValueTypes, parent?: KeyValue, optionMembers?: {
  members: Members,
  typeParameters: Members,
  methodParams: Members,
  constraint: Members,
  sameName: KeyValue[],
  dependOnGlobals: Set<KeyValue>
}): KeyValue {
  const keyValue = {key, type, parent, sameDeclares: []} as KeyValue;
  if (optionMembers) {
    keyValue.members = optionMembers.members ?? {};
    keyValue.typeParameters = optionMembers.typeParameters ?? {};
    keyValue.methodParams = optionMembers.methodParams ?? {};
    keyValue.constraint = optionMembers.constraint ?? {};
    keyValue.sameName = optionMembers.sameName ?? [];
    keyValue.dependOnGlobals = optionMembers.dependOnGlobals ?? new Set<KeyValue>();
  } else {
    keyValue.members = {};
    keyValue.typeParameters = {};
    keyValue.methodParams = {};
    keyValue.constraint = {};
    keyValue.sameName = [];
    keyValue.dependOnGlobals = new Set<KeyValue>();
  }
  return keyValue as KeyValue;
}

/**
 * 获取开源接口的api路径
 */
export function getOhosInterfacesDir(): string {
  return path.join(getProjectDir(), 'ets1.1', 'sdk-js', 'api');
}

/**
 * 获取导入路径的绝对路径
 * @param mockBuffer 文件mock缓存信息
 * @param specifier 导入路径
 * @returns 导入文件的绝对路径
 */
export function getAbsolutePath(mockBuffer: MockBuffer, specifier: string): string {
  let absolutePath: string;
  const importPath = specifier.replace(/['"]/g, '');
  if (importPath.startsWith('./') || importPath.startsWith('../')) {
    absolutePath = path.resolve(path.dirname(mockBuffer.mockedFilePath), importPath) + '.js';
  } else if (importPath.startsWith('@ohos.') || importPath.startsWith('@system.')) {
    absolutePath = path.join(NAPI_DIR_PATH, 'api', importPath) + '.js';
  } else if (importPath.startsWith('@kit.')) {
    absolutePath = path.join(NAPI_DIR_PATH, 'kits', importPath) + '.js';
  } else if (importPath.startsWith('@arkts.')) {
    absolutePath = path.join(NAPI_DIR_PATH, 'arkts', importPath) + '.js';
  } else {
    absolutePath = path.resolve(path.dirname(mockBuffer.mockedFilePath), importPath) + '.js';
  }
  return absolutePath;
}

/**
 * 处理迭代器函数
 * @param typeParams 迭代器返回值的类型参数
 */
export function handleIterableIterator(typeParams: string): string {
  return `function* () {
    const yieldObj = ${typeParams};
    yield yieldObj;  
    yield yieldObj;
    yield yieldObj;
  }`;
}

/**
 * 关联类型参数
 * @param keyValue KV节点
 * @param typeParameters
 */
export function associateTypeParameters(keyValue: KeyValue, typeParameters: Members): void {
  if (!keyValue.property) {
    keyValue.typeParameters = typeParameters;
    return;
  }
  associateTypeParameters(keyValue.property, typeParameters);
}

/**
 * 判断是否是类型申明文件
 * @param fileName 文件名
 */
export function isDeclarationFile(fileName: string): boolean {
  return fileName.endsWith(D_TS) || fileName.endsWith(D_ETS);
}

/**
 * 判断文件是否需要被mock
 * @param filePath 文件路径
 */
export function isNeedMocked(filePath: string): boolean {
  if (path.basename(filePath).startsWith('@ohos.')) {
    return true;
  }
  if (path.basename(filePath).startsWith('@arkts.')) {
    return true;
  }
  return path.basename(filePath).startsWith('@kit.');
}

/**
 * .d.ets 和 .d.ts文件同时存在时，过滤掉.d.ets文件
 * @param value 文件名称
 * @returns boolean
 */
export function identifyDuplicateFile(value: string, arr: string[]): boolean {
  const extName = path.extname(value);
  if (extName === '.ets' && arr.includes(value.replace('.ets', '.ts'))) {
    return true;
  }
  return false;
}

/**
 * 过滤掉不需要mock的文件
 * @param value 文件名称
 * @returns boolean
 */
export function filterDuplicateFile(value: string): boolean {
  for (let i = 0; i < needShieldingDirOrFile.length; i++) {
    if (value.includes(needShieldingDirOrFile[i])) {
      return true;
    }
  }
  return false;
}

/**
 * Determine whether the content contains arkts1.2 by parsing the comments.
 * @param node
 * @param sourceFile
 * @returns boolean
 */
export function isArktsOne(node: Node, sourceFile: SourceFile): boolean {
  const jsDocNode = node['jsDoc'] as JSDoc[];
  if (!jsDocNode) {
    return true;
  }
  for (let i = 0; i < jsDocNode.length; i++) {
    const element = jsDocNode[i];
    if (sourceFile.text.substring(element.pos, element.end).includes('@arkts 1.2')) {
      return false;
    }
  }
  return true;
}
