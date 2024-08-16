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
import {describe, expect, test} from '@jest/globals';
import { 
  getAllLegalImports,
  collectAllLegalImports,
  getAllFileNameList,
  collectAllFileName,
  getClassNameSet,
  getAllClassDeclaration,
  firstCharacterToUppercase,
} from '../common/commonUtils';
import { createSourceFile, ScriptTarget } from 'typescript';

function areSetsEqual(setA, setB) {
  if (setA.size !== setB.size) {
      return false; // 大小不同，返回 false
  }
  for (let item of setA) {
      if (!setB.has(item)) {
          return false; // setB 中没有 setA 的某个元素，返回 false
      }
  }
  return true; // 两个 set 相等
}

describe('commonUtils.ts file test', (): void => {
  test('Test the getAllLegalImports function', (): void => {
    collectAllLegalImports('hello world')
    const result = getAllLegalImports();
    const expectedResult = new Set<string>();
    expectedResult.add('hello world');
    expect(true).toBe(areSetsEqual(result, expectedResult));
  });

  test('Test the getAllFileNameList function', (): void => {
    collectAllFileName(path.join(__dirname, 'api/@ohos.ability.ability.d.ts'))
    collectAllFileName(path.join(__dirname, 'api/@ohos.ability.errorCode.d.ts'))
    collectAllFileName(path.join(__dirname, 'api/lifecycle.d.ts'))
    const result = getAllFileNameList();
    const expectedResult = new Set<string>();
    expectedResult.add('ohos_ability_ability');
    expectedResult.add('ohos_ability_errorCode');
    expectedResult.add('lifecycle');
    expect(true).toBe(areSetsEqual(result, expectedResult));
  });

  test('Test the getClassNameSet function', (): void => {
    const filePath = path.join(__dirname, './api/@ohos.accessibility.GesturePath.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    getAllClassDeclaration(sourceFile)
    const result = getClassNameSet();
    const expectedResult = new Set<string>();
    expectedResult.add('GesturePath');
    expect(true).toBe(areSetsEqual(result, expectedResult));
  });

  test('Test the getIsModuleDeclaration function', (): void => {
    const filePath = path.join(__dirname, './api/@ohos.accessibility.GesturePath.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    getAllClassDeclaration(sourceFile)
    const result = getClassNameSet();
    const expectedResult = new Set<string>();
    expectedResult.add('GesturePath');
    expect(true).toBe(areSetsEqual(result, expectedResult));
  });

  test('Test the getIsModuleDeclaration function', (): void => {
    const filePath = path.join(__dirname, './api/@ohos.account.appAccount.d.ts');
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    getAllClassDeclaration(sourceFile)
    const result = getClassNameSet();
    const expectedResult = new Set<string>();
    expectedResult.add('GesturePath');
    expectedResult.add('Authenticator');
    expect(true).toBe(areSetsEqual(result, expectedResult));
  });

  test('Test the firstCharacterToUppercase function', (): void => {
    const result = firstCharacterToUppercase('helloWorld');
    const expectedResult = 'HelloWorld';
    expect(result).toBe(expectedResult);
  });
});
