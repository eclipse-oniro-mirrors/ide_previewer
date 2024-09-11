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
import {
  getCallbackStatement,
  getReturnStatement,
  getWarnConsole,
  getReturnData,
  getOverloadedFunctionCallbackStatement,
  overloadedFunctionArr
} from './generateCommonUtil';
import { methodArrayItemForEach } from './generateCommonMethod';

interface MethodSignatureArrayProps {
  methodSignatureArray: Array<MethodSignatureEntity>;
  methodEntity: MethodSignatureEntity;
  methodSignatureBody: string;
  sourceFile: SourceFile;
  mockApi: string;
}

interface MethodSignatureArrayBack {
  methodSignatureArray: Array<MethodSignatureEntity>;
  returnSet: Set<string>;
  isCallBack: boolean;
  methodSignatureBody: string;
}

/**
 * generate interface signature method
 * @param rootName
 * @param methodSignatureArray
 * @param sourceFile
 * @returns
 */
export function generateCommonMethodSignature(
  rootName: string,
  methodSignatureArray: Array<MethodSignatureEntity>,
  sourceFile: SourceFile,
  mockApi: string
): string {
  let methodSignatureBody = '';
  const methodEntity = methodSignatureArray[0];
  methodSignatureBody += `${methodEntity.functionName}: function(...args) {`;
  methodSignatureBody += getWarnConsole(rootName, methodEntity.functionName);
  if (methodSignatureArray.length === 1) {
    const args = methodEntity.args;
    const len = args.length;
    if (len && args[len - 1].paramName.toLowerCase().includes('callback')) {
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
    const methodSignatureArrayBack = methodSignatureArrayForEach({
      methodSignatureArray,
      methodEntity,
      methodSignatureBody,
      sourceFile,
      mockApi
    });
    methodSignatureBody = returnSetForEach(methodSignatureArrayBack, sourceFile, mockApi);
  }
  methodSignatureBody += '},\n';
  return methodSignatureBody;
}

/**
 * returnSet ForEach
 * @param porps
 * @param sourceFile
 * @param mockApi
 * @returns
 */
function returnSetForEach(porps: MethodSignatureArrayBack, sourceFile: SourceFile, mockApi: string): string {
  let isReturnPromise = false;
  let promiseReturnValue = '';
  let methodSignatureOtherReturnValue = '';
  porps.returnSet.forEach(value => {
    if (value.includes('Promise<')) {
      isReturnPromise = true;
      promiseReturnValue = value;
    } else {
      if (!methodSignatureOtherReturnValue) {
        methodSignatureOtherReturnValue = value;
      }
    }
  });
  if (isReturnPromise) {
    if (promiseReturnValue) {
      let returnType = null;
      porps.methodSignatureArray.forEach(value => {
        if (value.returnType.returnKindName === promiseReturnValue) {
          returnType = value.returnType;
        }
      });
      porps.methodSignatureBody += getReturnData(porps.isCallBack, isReturnPromise, returnType, sourceFile, mockApi);
    } else {
      porps.methodSignatureBody += `
          return new Promise((resolve, reject) => {
            resolve('[PC Preview] unknow boolean');
          })
        `;
    }
  } else if (methodSignatureOtherReturnValue) {
    let returnType = null;
    porps.methodSignatureArray.forEach(value => {
      if (value.returnType.returnKindName === methodSignatureOtherReturnValue) {
        returnType = value.returnType;
      }
    });
    porps.methodSignatureBody += getReturnData(porps.isCallBack, isReturnPromise, returnType, sourceFile, mockApi);
  }
  return porps.methodSignatureBody;
}

/**
 * methodSignatureArray ForEach
 * @param porps
 * @returns
 */
function methodSignatureArrayForEach(porps: MethodSignatureArrayProps): MethodSignatureArrayBack {
  let argSet: Set<string> = new Set<string>();
  let argParamsSet: string = '';
  let returnSet: Set<string> = new Set<string>();
  let isCallBack = false;
  let needOverloaded = false;
  const methodSignatureArray = porps.methodSignatureArray;
  const sourceFile = porps.sourceFile;
  const mockApi = porps.mockApi;
  porps.methodSignatureArray.forEach(value => {
    ({ returnSet, argSet, isCallBack, argParamsSet, needOverloaded} =
      methodArrayItemForEach({returnSet, value, argSet, isCallBack, argParamsSet, needOverloaded}));
  });
  if (isCallBack) {
    if (overloadedFunctionArr.includes(porps.methodEntity.functionName) && needOverloaded) {
      porps.methodSignatureBody += getOverloadedFunctionCallbackStatement(methodSignatureArray, sourceFile, mockApi);
    } else {
      porps.methodSignatureBody += getCallbackStatement(porps.mockApi, argParamsSet);
    }
  }
  return {
    returnSet,
    isCallBack,
    methodSignatureBody: porps.methodSignatureBody,
    methodSignatureArray: porps.methodSignatureArray
  };
}
