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
import type { MethodEntity } from '../declaration-node/methodDeclaration';
import type { FunctionEntity } from '../declaration-node/functionDeclaration';
import type { MethodSignatureEntity } from '../declaration-node/methodSignatureDeclaration';
import {
  generateSymbolIterator,
  getCallbackStatement,
  getReturnStatement,
  getWarnConsole,
  getReturnData,
  getOverloadedFunctionCallbackStatement,
  overloadedFunctionArr
} from './generateCommonUtil';

interface MethodArrayProps {
  methodArray: Array<MethodEntity>,
  methodEntity: MethodEntity,
  sourceFile: SourceFile,
  mockApi: string,
  methodBody: string
}

interface MethodArrayItemForEachProps {
  returnSet: Set<string>,
  value: MethodEntity | FunctionEntity | MethodSignatureEntity,
  argSet: Set<string>,
  isCallBack: boolean,
  argParamsSet: string,
  needOverloaded: boolean
}

interface MethodArrayBack {
  returnSet: Set<string>,
  methodBody: string,
  isCallBack: boolean
}

/**
 * generate class method
 * @param rootName
 * @param methodArray
 * @param sourceFile
 * @returns
 */
export function generateCommonMethod(
  rootName: string,
  methodArray: Array<MethodEntity>,
  sourceFile: SourceFile,
  mockApi: string
): string {
  let methodBody = '';
  const methodEntity = methodArray[0];
  if (methodEntity.functionName.name === 'Symbol.iterator') {
    methodBody += `this[${methodEntity.functionName.name}] = function(...args) {`;
    methodBody += getWarnConsole(rootName, methodEntity.functionName.name);
    methodBody += generateSymbolIterator(methodEntity);
    methodBody += '};\n';
    return methodBody;
  } else {
    methodBody += `this.${methodEntity.functionName.name} = function(...args) {`;
    methodBody += getWarnConsole(rootName, methodEntity.functionName.name);
  }

  if (methodArray.length === 1) {
    const args = methodEntity.args;
    const len = args.length;
    if (args.length > 0 && args[len - 1].paramName.toLowerCase().includes('callback')) {
      methodBody += getCallbackStatement(mockApi, args[len - 1]?.paramTypeString);
    }
    if (methodEntity.returnType.returnKind !== SyntaxKind.VoidKeyword) {
      if (methodEntity.functionName.name === 'getApplicationContext') {
        methodBody += getApplicationContextValue(methodEntity.functionName.name);
      } else {
        methodBody += getReturnStatement(methodEntity.returnType, sourceFile);
      }
    }
  } else {
    const methodArrayBack = methodArrayForEach({ methodArray, methodEntity, sourceFile, mockApi, methodBody });
    methodBody = returnSetForEach(methodArrayBack, methodArray, sourceFile, mockApi);
  }
  methodBody += '};\n';
  return methodBody;
}

function getApplicationContextValue(name: string): string {
  return `
const mockData = {
  on: function (...args) {
    console.warn(
      'The ${name}.on interface in the Previewer is a mocked implementation and may behave differently than on a real device.'
    );
    if (args && ['environment'].includes(args[0])) {
      if (args && typeof args[args.length - 1] === 'function') {
        const EnvironmentCallback = mockEnvironmentCallback();
        args[args.length - 1].call(this, new EnvironmentCallback());
      }
    }
    return 0;
  },
  off: function (...args) {
    console.warn(
      'The ${name}.off interface in the Previewer is a mocked implementation and may behave differently than on a real device.'
    );
    if (args && ['environment'].includes(args[0])) {
      if (args && typeof args[args.length - 1] === 'function') {
        args[args.length - 1].call(
          this,
          { 'code': '', 'data': '', 'name': '', 'message': '', 'stack': '' },
          '[PC Preview] unknown type'
        );
      }
    } 
    return new Promise((resolve, reject) => {
        resolve('[PC Preview] unknown type');
    });
  }
};
return mockData;
  `;
}

/**
 * method Array ForEach
 * @param props
 * @returns
 */
function methodArrayForEach(props: MethodArrayProps): MethodArrayBack {
  let argSet: Set<string> = new Set<string>();
  let argParamsSet: string = '';
  let returnSet: Set<string> = new Set<string>();
  let isCallBack = false;
  let needOverloaded = false;
  props.methodArray.forEach(value => {
    ({ returnSet, argSet, isCallBack, argParamsSet, needOverloaded} =
      methodArrayItemForEach({returnSet, value, argSet, isCallBack, argParamsSet, needOverloaded}));
  });
  if (isCallBack) {
    if (overloadedFunctionArr.includes(props.methodEntity.functionName.name) && needOverloaded) {
      props.methodBody += getOverloadedFunctionCallbackStatement(props.methodArray, props.sourceFile, props.mockApi);
    } else {
      props.methodBody += getCallbackStatement(props.mockApi, argParamsSet);
    }
  }
  return {
    returnSet,
    methodBody: props.methodBody,
    isCallBack
  };
}

/**
 * method ArrayItem ForEach
 * @param props
 * @returns
 */
export function methodArrayItemForEach(
  props: MethodArrayItemForEachProps
): MethodArrayItemForEachProps {
  props.returnSet.add(props.value.returnType.returnKindName);
  props.value.args.forEach(arg => {
    props.argSet.add(arg.paramName);
    if (arg.paramName.toLowerCase().includes('callback')) {
      props.isCallBack = true;
      if (arg.paramTypeString) {
        props.argParamsSet = arg.paramTypeString;
      }
    }
    if (
      arg.paramTypeString.startsWith("'") && arg.paramTypeString.endsWith("'") ||
      arg.paramTypeString.startsWith('"') && arg.paramTypeString.endsWith('"')
    ) {
      props.needOverloaded = true;
    }
  });
  return props;
}

/**
 * returnSet ForEach
 * @param props
 * @param methodArray
 * @param sourceFile
 * @param mockApi
 * @returns
 */
function returnSetForEach(
  props: MethodArrayBack,
  methodArray: Array<MethodEntity>,
  sourceFile: SourceFile,
  mockApi: string
): string {
  let isReturnPromise = false;
  let promiseReturnValue = '';
  let methodOtherReturnValue = '';
  props.returnSet.forEach(value => {
    if (value.includes('Promise<')) {
      isReturnPromise = true;
      promiseReturnValue = value;
    } else {
      if (!methodOtherReturnValue) {
        methodOtherReturnValue = value;
      }
    }
  });
  if (isReturnPromise) {
    if (promiseReturnValue) {
      let returnType = null;
      methodArray.forEach(value => {
        if (value.returnType.returnKindName === promiseReturnValue) {
          returnType = value.returnType;
        }
      });
      props.methodBody += getReturnData(props.isCallBack, isReturnPromise, returnType, sourceFile, mockApi);
    } else {
      props.methodBody += `
            return new Promise((resolve, reject) => {
              resolve('[PC Preview] unknow boolean');
            })
          `;
    }
  } else if (methodOtherReturnValue) {
    let returnType = null;
    methodArray.forEach(value => {
      if (value.returnType.returnKindName === methodOtherReturnValue) {
        returnType = value.returnType;
      }
    });
    props.methodBody += getReturnData(props.isCallBack, isReturnPromise, returnType, sourceFile, mockApi);
  }
  return props.methodBody;
}
