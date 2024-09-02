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

import { isIdentifier } from 'typescript';
import type { SourceFile, VariableStatement } from 'typescript';

/**
 * get const declaration variable
 * @param variableStatement
 * @param sourceFile
 * @returns
 */
export function getVariableStatementDeclaration(
  variableStatement: VariableStatement,
  sourceFile: SourceFile
): Array<StatementEntity> {
  const statementsArray: Array<StatementEntity> = [];
  const fileText = sourceFile.getFullText();
  variableStatement.declarationList.declarations.forEach(value => {
    let statementName = '';
    let initializer = '';
    let typeName = '';
    let typeKind = -1;

    if (isIdentifier(value.name)) {
      statementName = value.name.escapedText.toString();
    } else {
      statementName = fileText.slice(value.pos, value.end).trim();
    }
    if (value.initializer !== undefined) {
      initializer = fileText.slice(value.initializer.pos, value.initializer.end);
      typeKind = value.initializer.kind;
    }
    if (value.type !== undefined) {
      typeName = fileText.slice(value.type.pos, value.type.end);
      typeKind = value.type.kind;
    }
    statementsArray.push({
      statementName: statementName,
      typeName: typeName,
      typeKind: typeKind,
      initializer: initializer
    });
  });
  return statementsArray;
}

export interface StatementEntity {
  statementName: string,
  typeName: string,
  typeKind: number,
  initializer: string
}
