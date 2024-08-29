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
import { getCallbackStatement, getReturnStatement } from './generateCommonUtil';

/**
 * generate function
 * @param rootName
 * @param functionArray
 * @param sourceFile
 * @param mockApi
 * @returns
 */
export function generateExportFunction(functionEntity: FunctionEntity, sourceFile: SourceFile, mockApi: string): string {
  let functionBody = '';
  functionBody = `const ${functionEntity.functionName} = function (...args) {`;
  if (mockApi.includes(functionBody)) {
    return '';
  }
  functionBody += `console.warn('The ${functionEntity.functionName} interface in the Previewer is a mocked implementation and may behave differently than on a real device.');\n`;

  const args = functionEntity.args;
  const len = args.length;
  if (len && args[len - 1].paramName.toLowerCase().includes('callback')) {
    functionBody += getCallbackStatement(mockApi);
  }
  if (functionEntity.returnType.returnKind !== SyntaxKind.VoidKeyword) {
    functionBody += getReturnStatement(functionEntity.returnType, sourceFile);
  }
  functionBody += '}';
  functionBody += `
    if (!global.${functionEntity.functionName}) {
      global.${functionEntity.functionName} = ${functionEntity.functionName}
    }
  `;
  return functionBody;
}

