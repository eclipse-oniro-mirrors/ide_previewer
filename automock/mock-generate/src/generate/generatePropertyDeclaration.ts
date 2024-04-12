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
import { getClassNameSet } from '../common/commonUtils';
import type { PropertyEntity } from '../declaration-node/propertyDeclaration';
import { getTheRealReferenceFromImport, getWarnConsole } from './generateCommonUtil';
import { ImportElementEntity } from '../declaration-node/importAndExportDeclaration';
import { addExtraImport } from './generateInterfaceDeclaration';

/**
 * generate class property
 * @param rootName
 * @param propertyDeclaration
 * @param sourceFile
 * @returns
 */
export function generatePropertyDeclaration(rootName: string, propertyDeclaration: PropertyEntity, sourceFile: SourceFile, extraImport: string[], importDeclarations?: ImportElementEntity[]): string {
  let propertyBody = '';
  if (propertyDeclaration.isInitializer) {
    propertyBody = `this.${propertyDeclaration.propertyName} = ${propertyDeclaration.initializer};`;
  } else {
    propertyBody = `this.${propertyDeclaration.propertyName} = `;
    if (propertyDeclaration.kinds === SyntaxKind.GetAccessor) {
      const warnCon = getWarnConsole(rootName, propertyDeclaration.propertyName);
      propertyBody += `(function () {\n ${warnCon} \n return `;
    }
    if (propertyDeclaration.propertyTypeName.startsWith('{')) {
      propertyBody += '{};';
    } else if (propertyDeclaration.kind === SyntaxKind.LiteralType) {
      propertyBody += `${propertyDeclaration.propertyTypeName};`;
    } else if (propertyDeclaration.kind === SyntaxKind.NumberKeyword) {
      propertyBody += '0;';
    } else if (propertyDeclaration.kind === SyntaxKind.StringKeyword) {
      propertyBody += '\'\'';
    } else if (propertyDeclaration.kind === SyntaxKind.BooleanKeyword) {
      propertyBody += 'true';
    } else if (propertyDeclaration.propertyTypeName.startsWith('Array')) {
      propertyBody += '[];';
    } else if (propertyDeclaration.propertyTypeName.startsWith('Map')) {
      propertyBody += '{key: {}};';
    } else if (propertyDeclaration.kind === SyntaxKind.TypeReference) {
      if (getClassNameSet().has(propertyDeclaration.propertyTypeName)) {
        if (!['Want', 'InputMethodExtensionContext'].includes(propertyDeclaration.propertyTypeName)) {
          propertyBody += `new ${getTheRealReferenceFromImport(sourceFile, propertyDeclaration.propertyTypeName)}();`;
        } else {
          propertyBody += `${getTheRealReferenceFromImport(sourceFile, propertyDeclaration.propertyTypeName)};`;
        }
      } else {
        propertyBody += `${getTheRealReferenceFromImport(sourceFile, propertyDeclaration.propertyTypeName)};`;
      }
    } else if (propertyDeclaration.kind === SyntaxKind.NumericLiteral || propertyDeclaration.kind === SyntaxKind.StringLiteral) {
      propertyBody += ` ${propertyDeclaration.propertyTypeName};`;
    } else {
      propertyBody += `'[PC Previwe] unknown ${propertyDeclaration.propertyName}';`;
    }
    if (propertyDeclaration.kinds === SyntaxKind.GetAccessor) {
      addExtraImport(extraImport, importDeclarations, sourceFile, propertyDeclaration);
      propertyBody += '\n })();';
    }
  }
  return propertyBody;
}
