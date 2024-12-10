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

import ts, {
  isClassDeclaration,
  isEnumDeclaration,
  isExportAssignment,
  isExportDeclaration,
  isFunctionDeclaration,
  isImportDeclaration,
  isInterfaceDeclaration,
  isModuleDeclaration,
  isTypeAliasDeclaration,
  isVariableStatement,
  SourceFile
} from 'typescript';
import { KeyValue, Members, MockBuffer } from '../types';
import { KeyValueTypes } from '../common/constants';
import {
  handleClassDeclaration,
  handleEnumDeclaration,
  handleExportAssignment,
  handleExportDeclaration,
  handleFunctionDeclaration,
  handleImportDeclaration,
  handleInterfaceDeclaration,
  handleModuleDeclaration,
  handleTypeAliasDeclaration,
  handleVariableStatement
} from '../common/tsNodeUtils';

/**
 * 解析所有 sourceFile 节点信息
 * @param sourceFile sourceFile对象
 * @param fileName 文件路径
 * @returns
 */
export function getSourceFileAssembly(sourceFile: SourceFile, mockBuffer: MockBuffer, members: Members, parent: KeyValue): void {
  let defaultExportNode: ts.ExportAssignment;
  sourceFile.forEachChild(node => {
    if (isImportDeclaration(node)) {
      handleImportDeclaration(node, mockBuffer, members, parent, KeyValueTypes.IMPORT);
    } else if (isModuleDeclaration(node)) {
      handleModuleDeclaration(node, mockBuffer, members, parent, KeyValueTypes.MODULE);
    } else if (isTypeAliasDeclaration(node)) {
      handleTypeAliasDeclaration(node, mockBuffer, members, parent, KeyValueTypes.VARIABLE);
    } else if (isClassDeclaration(node)) {
      handleClassDeclaration(node, mockBuffer, members, parent, KeyValueTypes.CLASS);
    } else if (isInterfaceDeclaration(node)) {
      handleInterfaceDeclaration(node, mockBuffer, members, parent, KeyValueTypes.INTERFACE);
    } else if (isEnumDeclaration(node)) {
      handleEnumDeclaration(node, mockBuffer, members, parent, KeyValueTypes.ENUM);
    } else if (isFunctionDeclaration(node)) {
      handleFunctionDeclaration(node, mockBuffer, members, parent, KeyValueTypes.FUNCTION);
    } else if (isExportAssignment(node)) {
      defaultExportNode = node;
    } else if (isExportDeclaration(node)) {
      handleExportDeclaration(node, mockBuffer, members, parent, KeyValueTypes.IMPORT);
    } else if (isVariableStatement(node)) {
      handleVariableStatement(node, mockBuffer, members, parent, KeyValueTypes.VARIABLE);
    }
  });
  defaultExportNode && handleExportAssignment(defaultExportNode, mockBuffer, members, parent, KeyValueTypes.EXPORT);
}
