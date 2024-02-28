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

import {
  isClassDeclaration, isEnumDeclaration, isExportDeclaration, isFunctionDeclaration, isIdentifier,
  isImportEqualsDeclaration, isInterfaceDeclaration, isModuleBlock, isModuleDeclaration, isTypeAliasDeclaration,
  isVariableStatement, ModuleBody
} from 'typescript';
import type { ModuleDeclaration, Node, SourceFile } from 'typescript';
import { getExportKeyword } from '../common/commonUtils';
import { getClassDeclaration } from './classDeclaration';
import type { ClassEntity } from './classDeclaration';
import { getEnumDeclaration } from './enumDeclaration';
import type { EnumEntity } from './enumDeclaration';
import { getFunctionDeclaration } from './functionDeclaration';
import type { FunctionEntity } from './functionDeclaration';
import { getExportDeclaration, getModuleImportEqual } from './importAndExportDeclaration';
import type { ImportEuqalEntity } from './importAndExportDeclaration';
import { getInterfaceDeclaration } from './interfaceDeclaration';
import type { InterfaceEntity } from './interfaceDeclaration';
import { getTypeAliasDeclaration } from './typeAliasDeclaration';
import type { TypeAliasEntity } from './typeAliasDeclaration';
import { getVariableStatementDeclaration } from './variableStatementResolve';
import type { StatementEntity } from './variableStatementResolve';

interface SubstepGetModuleparam {
  typeAliasDeclarations: Array<TypeAliasEntity>,
  classDeclarations: Array<ClassEntity>,
  interfaceDeclarations: Array<InterfaceEntity>,
  functionDeclarations: Map<string, Array<FunctionEntity>>,
  enumDeclarations: Array<EnumEntity>,
  moduleDeclarations: Array<ModuleBlockEntity>,
  variableStatements: Array<Array<StatementEntity>>,
  moduleImportEquaqls: Array<ImportEuqalEntity>,
  exportDeclarations: Array<string>,
  sourceFile: SourceFile,
  fileName: string,
  moduleBody: ModuleBody
}

export interface ModuleBlockEntity {
  moduleName: string,
  exportModifiers: Array<number>,
  typeAliasDeclarations: Array<TypeAliasEntity>,
  classDeclarations: Array<ClassEntity>,
  interfaceDeclarations: Array<InterfaceEntity>,
  functionDeclarations: Map<string, Array<FunctionEntity>>,
  enumDeclarations: Array<EnumEntity>,
  moduleDeclarations: Array<ModuleBlockEntity>,
  variableStatements: Array<Array<StatementEntity>>,
  moduleImportEquaqls: Array<ImportEuqalEntity>,
  exportDeclarations: Array<string>
}

/**
 * get module info
 * @param node
 * @param sourceFile
 * @param fileName
 * @returns
 */
export function getModuleDeclaration(node: Node, sourceFile: SourceFile, fileName: string): ModuleBlockEntity {
  const moduleNode = node as ModuleDeclaration;
  const typeAliasDeclarations: Array<TypeAliasEntity> = [];
  const classDeclarations: Array<ClassEntity> = [];
  const interfaceDeclarations: Array<InterfaceEntity> = [];
  const functionDeclarations: Map<string, Array<FunctionEntity>> = new Map<string, Array<FunctionEntity>>();
  const enumDeclarations: Array<EnumEntity> = [];
  const moduleDeclarations: Array<ModuleBlockEntity> = [];
  const variableStatements: Array<Array<StatementEntity>> = [];
  const moduleImportEquaqls: Array<ImportEuqalEntity> = [];
  const exportDeclarations: Array<string> = [];
  const moduleBody = moduleNode.body;
  let moduleName = '';
  if (isIdentifier(moduleNode.name)) {
    moduleName = moduleNode.name.escapedText.toString();
  } else {
    moduleName = sourceFile.text.substring(moduleNode.name.pos, moduleNode.name.end).trim();
  }

  let exportModifiers: Array<number> = [];
  const modifiers = moduleNode.modifiers;
  if (modifiers !== undefined) {
    exportModifiers = getExportKeyword(modifiers);
  }
  const SubstepModuleBlockEntitys: SubstepGetModuleparam = substepModule({
    typeAliasDeclarations,
    classDeclarations,
    interfaceDeclarations,
    functionDeclarations,
    enumDeclarations,
    moduleDeclarations,
    variableStatements,
    moduleImportEquaqls,
    exportDeclarations,
    moduleBody,
    sourceFile,
    fileName
  });
  delete SubstepModuleBlockEntitys.moduleBody;
  delete SubstepModuleBlockEntitys.sourceFile;
  delete SubstepModuleBlockEntitys.fileName;
  return {
    ...SubstepModuleBlockEntitys,
    exportModifiers,
    moduleName
  };
}

/**
 * get some module info
 * @param SubstepGetModuleparam
 * @returns
 */
function substepModule(props: SubstepGetModuleparam): SubstepGetModuleparam {
  if (props.moduleBody !== undefined && isModuleBlock(props.moduleBody)) {
    props.moduleBody.statements.forEach(value => {
      if (isFunctionDeclaration(value)) {
        const functionEntity = getFunctionDeclaration(value, props.sourceFile);
        if (props.functionDeclarations.get(functionEntity.functionName) !== undefined) {
          props.functionDeclarations.get(functionEntity.functionName)?.push(functionEntity);
        } else {
          const functionArray: Array<FunctionEntity> = [];
          functionArray.push(functionEntity);
          props.functionDeclarations.set(functionEntity.functionName, functionArray);
        }
      } else if (isTypeAliasDeclaration(value)) {
        props.typeAliasDeclarations.push(getTypeAliasDeclaration(value, props.sourceFile));
      } else if (isEnumDeclaration(value)) {
        props.enumDeclarations.push(getEnumDeclaration(value, props.sourceFile));
      } else if (isClassDeclaration(value)) {
        props.classDeclarations.push(getClassDeclaration(value, props.sourceFile));
      } else if (isInterfaceDeclaration(value)) {
        props.interfaceDeclarations.push(getInterfaceDeclaration(value, props.sourceFile));
      } else if (isModuleDeclaration(value)) {
        props.moduleDeclarations.push(getModuleDeclaration(value, props.sourceFile, props.fileName));
      } else if (isVariableStatement(value)) {
        props.variableStatements.push(getVariableStatementDeclaration(value, props.sourceFile));
      } else if (isImportEqualsDeclaration(value)) {
        props.moduleImportEquaqls.push(getModuleImportEqual(value, props.sourceFile));
      } else if (isExportDeclaration(value)) {
        props.exportDeclarations.push(getExportDeclaration(value, props.sourceFile));
      } else {
        console.log('--------------------------- uncaught module type start -----------------------');
        console.log('fileName: ' + props.fileName);
        console.log(value);
        console.log('--------------------------- uncaught module type end -----------------------');
      }
    });
  }
  return props;
}

interface SubstepGetModuleparam {
  typeAliasDeclarations: Array<TypeAliasEntity>,
  classDeclarations: Array<ClassEntity>,
  interfaceDeclarations: Array<InterfaceEntity>,
  functionDeclarations: Map<string, Array<FunctionEntity>>,
  enumDeclarations: Array<EnumEntity>,
  moduleDeclarations: Array<ModuleBlockEntity>,
  variableStatements: Array<Array<StatementEntity>>,
  moduleImportEquaqls: Array<ImportEuqalEntity>,
  exportDeclarations: Array<string>,
  sourceFile: SourceFile,
  fileName: string,
  moduleBody: ModuleBody
}

export interface ModuleBlockEntity {
  moduleName: string,
  exportModifiers: Array<number>,
  typeAliasDeclarations: Array<TypeAliasEntity>,
  classDeclarations: Array<ClassEntity>,
  interfaceDeclarations: Array<InterfaceEntity>,
  functionDeclarations: Map<string, Array<FunctionEntity>>,
  enumDeclarations: Array<EnumEntity>,
  moduleDeclarations: Array<ModuleBlockEntity>,
  variableStatements: Array<Array<StatementEntity>>,
  moduleImportEquaqls: Array<ImportEuqalEntity>,
  exportDeclarations: Array<string>
}
