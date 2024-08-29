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
import { getClassNameSet } from '../common/commonUtils';
import type { StatementEntity } from '../declaration-node/variableStatementResolve';

/**
 * generate const variable statement
 * @param statementEntity
 * @returns
 */
export function generateVariableStatementDelcatation(statementEntity: StatementEntity, isInnerModule: boolean): string {
  let statementBody = '';
  if (isInnerModule) {
    statementBody = `const ${statementEntity.statementName} = `;
  } else {
    statementBody = `${statementEntity.statementName}: `;
  }
  let statementValue: string;
  if (statementEntity.typeKind === SyntaxKind.StringKeyword) {
    statementValue = '\'\'';
  } else if (statementEntity.typeKind === SyntaxKind.LiteralType || statementEntity.typeKind === SyntaxKind.StringLiteral ||
    statementEntity.typeKind === SyntaxKind.NumericLiteral) {
    statementValue = judgmentStatementEntity(statementEntity, statementValue);
  } else if (statementEntity.typeKind === SyntaxKind.NumberKeyword) {
    statementValue = '0';
  } else if (statementEntity.typeKind === SyntaxKind.UnionType) {
    statementValue = statementEntity.typeName.split('|')[0];
  } else if (statementEntity.typeKind === SyntaxKind.TypeReference) {
    statementValue = judgmentStatementEntityTypeName(statementEntity, statementValue);
  } else if (statementEntity.typeKind === SyntaxKind.BooleanKeyword) {
    statementValue = 'true';
  } else {
    statementValue = `'[PC Preivew] unknown ${statementEntity.statementName}'`;
  }
  statementBody += statementValue;
  if (isInnerModule) {
    statementBody += ';';
  } else {
    statementBody += ',';
  }
  return statementBody;
}

function judgmentStatementEntity(statementEntity: StatementEntity, statementValue: string): string {
  if (statementEntity.initializer === '') {
    if (statementEntity.typeName.endsWith('n')) {
      statementValue = statementEntity.typeName.replace('n', '');
    } else {
      statementValue = statementEntity.typeName;
    }
  } else {
    statementValue = statementEntity.initializer;
  }
  return statementValue;
}
function judgmentStatementEntityTypeName(statementEntity: StatementEntity, statementValue: string): string {
  if (statementEntity.typeName.includes('<')) {
    const tmpTypeName = statementEntity.typeName.split('<')[0];
    if (getClassNameSet().has(tmpTypeName)) {
      statementValue = `new ${tmpTypeName}()`;
    } else {
      statementValue = `${tmpTypeName}`;
    }
  } else {
    statementValue = statementEntity.typeName;
  }
  return statementValue;
}
