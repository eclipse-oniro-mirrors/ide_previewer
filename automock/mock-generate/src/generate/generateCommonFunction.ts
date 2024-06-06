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
import type { FunctionEntity } from '../declaration-node/functionDeclaration';
import {
  getCallbackStatement,
  getReturnStatement,
  getWarnConsole,
  getReturnData,
  getOverloadedFunctionCallbackStatement,
  overloadedFunctionArr
} from './generateCommonUtil';
import { methodArrayItemForEach } from './generateCommonMethod';

interface AssemblyFunctionProps {
  functionArray: Array<FunctionEntity>,
  functionEntity: FunctionEntity,
  functionBody: string,
  sourceFile: SourceFile,
  mockApi: string
}

interface AssemblyFunctionBack {
  isReturnPromise: boolean,
  promiseReturnValue: string,
  functionOtherReturnValue: string,
  isCallBack: boolean,
  functionBody: string
}

/**
 * generate function
 * @param rootName
 * @param functionArray
 * @param sourceFile
 * @param mockApi
 * @param isRoot
 * @returns
 */
export function generateCommonFunction(
  rootName: string,
  functionArray: Array<FunctionEntity>,
  sourceFile: SourceFile,
  mockApi: string,
  isRoot: boolean
): string {
  let functionBody = '';
  const functionEntity = functionArray[0];
  if (isRoot) {
    functionBody = `const ${functionEntity.functionName} = function(...args) {`;
  } else {
    functionBody = `${functionEntity.functionName}: function(...args) {`;
  }
  functionBody += getWarnConsole(rootName, functionEntity.functionName);

  if (functionArray.length === 1) {
    const args = functionEntity.args;
    const len = args.length;
    if (args.length > 0 && args[len - 1].paramName.toLowerCase().includes('callback')) {
      functionBody += getCallbackStatement(mockApi, args[len - 1]?.paramTypeString);
    }
    if (functionEntity.returnType.returnKind !== SyntaxKind.VoidKeyword) {
      if (rootName === 'featureAbility' && functionEntity.returnType.returnKindName === 'Context') {
        functionBody += 'return _Context;';
      } else if (rootName === 'inputMethod' && functionEntity.returnType.returnKindName === 'InputMethodSubtype') {
        functionBody += 'return mockInputMethodSubtype().InputMethodSubtype;';
      } else {
        functionBody += getReturnStatement(functionEntity.returnType, sourceFile);
      }
    }
  } else {
    const assemblyBack = assemblyFunctionBody({ functionArray, functionEntity, functionBody, mockApi, sourceFile });
    functionBody = assemblyFuntion(assemblyBack, functionArray, sourceFile, mockApi);
    functionBody = assemblyBack.functionBody;
  }
  functionBody += isRoot ? '};' : '},';
  if (isRoot) {
    functionBody += `
      if (!global.${functionEntity.functionName}) {
        global.${functionEntity.functionName} = ${functionEntity.functionName};
      }
    `;
  }
  return functionBody;
}

/**
 * generate assembly function
 * @param props
 * @returns
 */
function assemblyFunctionBody(props: AssemblyFunctionProps): AssemblyFunctionBack {
  let argSet: Set<string> = new Set<string>();
  let argParamsSet: string = '';
  let returnSet: Set<string> = new Set<string>();
  let isCallBack = false;
  let needOverloaded = false;
  props.functionArray.forEach(value => {
    ({ returnSet, argSet, isCallBack, argParamsSet, needOverloaded} =
      methodArrayItemForEach({returnSet, value, argSet, isCallBack, argParamsSet, needOverloaded}));
  });
  props.functionBody = forEachFuntionArray(isCallBack, props, needOverloaded, argParamsSet);
  let isReturnPromise = false;
  let promiseReturnValue = '';
  let functionOtherReturnValue = '';
  returnSet.forEach(value => {
    if (value.includes('Promise<')) {
      isReturnPromise = true;
      promiseReturnValue = value;
    } else {
      if (!functionOtherReturnValue) {
        functionOtherReturnValue = value;
      }
    }
  });
  return {
    isReturnPromise,
    promiseReturnValue,
    functionOtherReturnValue,
    isCallBack,
    functionBody: props.functionBody
  };
}

/**
 * forEach functionArray
 * @param isCallBack
 * @param props
 * @param needOverloaded
 * @param argParamsSet
 * @returns
 */
function forEachFuntionArray(
  isCallBack: boolean,
  props: AssemblyFunctionProps,
  needOverloaded: boolean,
  argParamsSet:string
): string {
  if (isCallBack) {
    if (overloadedFunctionArr.includes(props.functionEntity.functionName) && needOverloaded) {
      const stateEment = getOverloadedFunctionCallbackStatement(props.functionArray, props.sourceFile, props.mockApi);
      props.functionBody += stateEment;
    } else {
      props.functionBody += getCallbackStatement(props.mockApi, argParamsSet);
    }
  }
  return props.functionBody;
}

/**
 * assembly Function
 * @param porps
 * @param functionArray
 * @param sourceFile
 * @param mockApi
 * @returns
 */
function assemblyFuntion(
  porps: AssemblyFunctionBack,
  functionArray: Array<FunctionEntity>,
  sourceFile: SourceFile,
  mockApi: string
): string {
  if (porps.isReturnPromise) {
    if (porps.promiseReturnValue) {
      let returnType = null;
      functionArray.forEach(value => {
        if (value.returnType.returnKindName === porps.promiseReturnValue) {
          returnType = value.returnType;
        }
      });
      porps.functionBody += getReturnData(porps.isCallBack, porps.isReturnPromise, returnType, sourceFile, mockApi);
    } else {
      porps.functionBody += `
          return new Promise((resolve, reject) => {
            resolve('[PC Preview] unknow boolean');
          })
        `;
    }
  } else if (porps.functionOtherReturnValue) {
    let returnType = null;
    functionArray.forEach(value => {
      if (value.returnType.returnKindName === porps.functionOtherReturnValue) {
        returnType = value.returnType;
      }
    });
    porps.functionBody += getReturnData(porps.isCallBack, porps.isReturnPromise, returnType, sourceFile, mockApi);
  }
  return porps.functionBody;
}

