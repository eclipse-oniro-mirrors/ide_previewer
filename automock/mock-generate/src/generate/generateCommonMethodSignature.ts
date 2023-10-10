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

import type { SourceFile } from 'typescript';
import { SyntaxKind } from 'typescript';
import type { MethodSignatureEntity } from '../declaration-node/methodSignatureDeclaration';
import { getCallbackStatement, getReturnStatement, getWarnConsole, getReturnData } from './generateCommonUtil';

/**
 * generate interface signature method
 * @param rootName
 * @param methodSignatureArray
 * @param sourceFile
 * @returns
 */
export function generateCommonMethodSignature(rootName: string, methodSignatureArray: Array<MethodSignatureEntity>, sourceFile: SourceFile, mockApi: string): string {
  let methodSignatureBody = '';
  const methodEntity = methodSignatureArray[0];
  methodSignatureBody += `${methodEntity.functionName}: function(...args) {`;
  methodSignatureBody += getWarnConsole(rootName, methodEntity.functionName);
  if (methodSignatureArray.length === 1) {
    const args = methodEntity.args;
    const len = args.length;
    if (args.length > 0 && args[len - 1].paramName.toLowerCase().includes('callback')) {
      methodSignatureBody += getCallbackStatement(mockApi, args[len - 1]?.paramTypeString);
    }
    if (methodEntity.returnType.returnKind !== SyntaxKind.VoidKeyword) {
      if (rootName === 'Context' && methodEntity.returnType.returnKindName === 'Context') {
        methodSignatureBody += 'return Context;';
      } else {
        methodSignatureBody += getReturnStatement(methodEntity.returnType, sourceFile);
      }
    }
  } else {
    const argSet: Set<string> = new Set<string>();
    let argParamsSet: string = '';
    const returnSet: Set<string> = new Set<string>();
    let isCallBack = false;
    methodSignatureArray.forEach(value => {
      returnSet.add(value.returnType.returnKindName);
      value.args.forEach(arg => {
        argSet.add(arg.paramName);
        if (arg.paramName.toLowerCase().includes('callback')) {
          isCallBack = true;
          if (arg.paramTypeString) {
            argParamsSet = arg.paramTypeString;
          }
        }
      });
    });
    if (isCallBack) {
      methodSignatureBody += getCallbackStatement(mockApi, argParamsSet);
    }
    let isReturnPromise = false;
    let promiseReturnValue = '';
    let otherReturnValue = '';
    returnSet.forEach(value => {
      if (value.includes('Promise<')) {
        isReturnPromise = true;
        promiseReturnValue = value;
      } else {
        if (!otherReturnValue) {
          otherReturnValue = value;
        }
      }
    });
    if (isReturnPromise) {
      if (promiseReturnValue) {
        let returnType = null;
        methodSignatureArray.forEach(value => {
          if (value.returnType.returnKindName === promiseReturnValue) {
            returnType = value.returnType;
          }
        });
        methodSignatureBody += getReturnData(isCallBack, isReturnPromise, returnType, sourceFile, mockApi);
      } else {
        if (isCallBack) {
          methodSignatureBody += `else {
              return new Promise((resolve, reject) => {
                resolve('[PC Preview] unknow boolean');
              })
            }`;
        } else {
          methodSignatureBody += `
              return new Promise((resolve, reject) => {
                resolve('[PC Preview] unknow boolean');
              })
            `;
        }
      }
    } else if (otherReturnValue) {
      let returnType = null;
      methodSignatureArray.forEach(value => {
        if (value.returnType.returnKindName === otherReturnValue) {
          returnType = value.returnType;
        }
      });
      methodSignatureBody += getReturnData(isCallBack, isReturnPromise, returnType, sourceFile, mockApi);
    }
  }
  methodSignatureBody += '},\n';
  return methodSignatureBody;
}
