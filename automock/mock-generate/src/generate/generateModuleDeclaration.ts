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

import path from 'path';
import { SyntaxKind } from 'typescript';
import type { SourceFile } from 'typescript';
import { firstCharacterToUppercase } from '../common/commonUtils';
import type { ModuleBlockEntity } from '../declaration-node/moduleDeclaration';
import {
  getDefaultExportClassDeclaration,
  getSourceFileFunctions,
  getSourceFileVariableStatements
} from '../declaration-node/sourceFileElementsAssemply';
import { generateClassDeclaration } from './generateClassDeclaration';
import { generateCommonFunction } from './generateCommonFunction';
import { generateEnumDeclaration } from './generateEnumDeclaration';
import { generateImportEqual } from './generateImportEqual';
import { addToIndexArray } from './generateIndex';
import { generateInterfaceDeclaration } from './generateInterfaceDeclaration';
import { generateStaticFunction } from './generateStaticFunction';
import { addToSystemIndexArray } from './generateSystemIndex';
import { generateTypeAliasDeclaration } from './generateTypeAlias';
import { generateVariableStatementDelcatation } from './generateVariableStatementDeclaration';
import type { ImportElementEntity } from '../declaration-node/importAndExportDeclaration';
import { ClassEntity } from '../declaration-node/classDeclaration';

interface ModuleExportEntity {
  type: string;
  name: string;
}

interface DefaultExportClassProps {
  moduleBody: string;
  outBody: string;
  filename: string;
  sourceFile: SourceFile;
  mockApi: string;
}

interface DefaultExportClassBack {
  moduleBody: string;
  outBody: string;
}

interface JudgmentModuleEntityProps {
  moduleEntity: ModuleBlockEntity;
  moduleBody: string;
  outBody: string;
  enumBody: string;
  sourceFile: SourceFile;
  mockApi: string;
  extraImport: string[];
  moduleName: string;
  importDeclarations: ImportElementEntity[];
}

interface JudgmentModuleEntityBack {
  moduleBody: string;
  outBody: string;
  enumBody: string;
}

interface ModuleEntityLoopProps {
  moduleEntity: ModuleBlockEntity;
  innerOutBody: string;
  moduleBody: string;
  sourceFile: SourceFile;
  mockApi: string;
  extraImport: string[];
  innerModuleName: string;
  importDeclarations: ImportElementEntity[];
}

interface ModuleEntityLoopBack {
  innerOutBody: string;
  moduleBody: string;
}

interface ModuleEntityNextProps {
  moduleEntity: ModuleBlockEntity;
  innerFunctionBody: string;
  innerModuleBody: string;
  filename: string;
  moduleBody: string;
  sourceFile: SourceFile;
  mockApi: string;
  extraImport: string[];
  innerModuleName: string;
  importDeclarations: ImportElementEntity[];
}

interface ModuleEntityNextBack {
  innerModuleName: string;
  moduleBody: string;
}

/**
 * generate declare
 * @param moduleEntity
 * @param sourceFile
 * @param filename
 * @param extraImport
 * @returns
 */
export function generateModuleDeclaration(
  moduleEntity: ModuleBlockEntity,
  sourceFile: SourceFile,
  filename: string,
  mockApi: string,
  extraImport: string[],
  importDeclarations: ImportElementEntity[]
): string {
  const innerModuleBody = '';
  const moduleName = moduleEntity.moduleName.replace(/["']/g, '');
  let moduleBody = `export function mock${firstCharacterToUppercase(moduleName)}() {\n`;
  let enumBody = '';
  if (
    !(
      moduleEntity.exportModifiers.includes(SyntaxKind.DeclareKeyword) &&
      (moduleEntity.moduleName.startsWith('"') || moduleEntity.moduleName.startsWith('\''))
    ) &&
    path.basename(sourceFile.fileName).startsWith('@ohos')
  ) {
    addToIndexArray({ fileName: filename, mockFunctionName: `mock${firstCharacterToUppercase(moduleName)}` });
  }
  let outBody = '';
  const defaultExportClassBack = defaultExportClassForEach({ moduleBody, outBody, filename, sourceFile, mockApi });
  moduleBody = defaultExportClassBack.moduleBody;
  outBody = defaultExportClassBack.outBody;
  const judgmentModuleEntityProps = {
    moduleEntity,
    moduleBody: defaultExportClassBack.moduleBody,
    outBody: defaultExportClassBack.outBody,
    sourceFile,
    mockApi,
    enumBody,
    extraImport,
    moduleName,
    importDeclarations
  };
  const judgmentModuleEntityBack = judgmentModuleEntity(judgmentModuleEntityProps);
  moduleBody = judgmentModuleEntityBack.moduleBody;
  outBody = judgmentModuleEntityBack.outBody;
  enumBody = judgmentModuleEntityBack.enumBody;
  moduleBody = moduleEntityForEach(judgmentModuleEntityProps, innerModuleBody, filename);

  const exportString = getExportBody(moduleEntity);
  if (exportString !== '') {
    moduleBody += '\t' + exportString;
  }

  moduleBody += '\t};';
  moduleBody += `\n\treturn ${moduleName};}\n`;
  moduleBody += outBody;
  moduleBody = enumBody + moduleBody;
  return moduleBody;
}

/**
 * Obtain the variables to be exported.
 *
 * @param moduleEntity ModuleBlockEntity
 * @returns string
 */
function getExportBody(moduleEntity: ModuleBlockEntity): string {
  let exportString = '';
  const exports = getModuleExportElements(moduleEntity);
  exports.forEach(value => {
    if (value.type === 'module' && !value.name.startsWith("'") && !value.name.startsWith('"')) {
      exportString += `${value.name}: mock${value.name}(),\n`;
    } else {
      exportString += `${value.name}: ${value.name},\n`;
    }
  });
  return exportString;
}

/**
 * judgment ModuleEntityLength
 * @param props
 * @param innerModuleBody
 * @param filename
 * @returns
 */
function moduleEntityForEach(props: JudgmentModuleEntityProps, innerModuleBody: string, filename: string): string {
  let functionBody = '';
  if (props.moduleEntity.functionDeclarations.size > 0) {
    props.moduleEntity.functionDeclarations.forEach(value => {
      functionBody += '\t' + generateCommonFunction(props.moduleName, value, props.sourceFile,
        props.mockApi, false) + '\n';
    });
  }
  if (props.moduleEntity.moduleDeclarations.length > 0) {
    props.moduleEntity.moduleDeclarations.forEach(value => {
      if (!value.moduleName.startsWith("'") && !value.moduleName.startsWith('"')) {
        innerModuleBody += generateInnerModuleDeclaration(value, props.sourceFile, filename, props.mockApi,
          props.extraImport, props.importDeclarations);
      }
    });
  }
  if (innerModuleBody) {
    props.moduleBody += innerModuleBody + '\n';
  }
  props.moduleBody += '\t' + `const ${props.moduleName} = {`;
  if (props.moduleEntity.variableStatements.length > 0) {
    props.moduleEntity.variableStatements.forEach(value => {
      value.forEach(val => {
        props.moduleBody += generateVariableStatementDelcatation(val, false) + '\n';
      });
    });
  }
  const sourceFileFunctions = getSourceFileFunctions(props.sourceFile);
  let sourceFileFunctionBody = '';
  if (sourceFileFunctions.size > 0) {
    sourceFileFunctions.forEach(value => {
      sourceFileFunctionBody += '\n' + generateCommonFunction(props.moduleName, value,
        props.sourceFile, props.mockApi, false);
    });
  }
  const sourceFileVariableStatements = getSourceFileVariableStatements(props.sourceFile);
  let sourceFileStatementBody = '';
  if (sourceFileVariableStatements.length > 0) {
    sourceFileVariableStatements.forEach(value => {
      value.forEach(val => {
        sourceFileStatementBody += '\n' + generateVariableStatementDelcatation(val, false);
      });
    });
  }
  props.moduleBody += sourceFileFunctionBody + '\n';
  props.moduleBody += sourceFileStatementBody + '\n';
  props.moduleBody += functionBody + '\n';
  return props.moduleBody;
}

/**
 * handle extra class declaration body
 * @param value
 * @param fileName
 * @returns
 */
function handleExtraClassDeclarationBody(value: ClassEntity, fileName: string): boolean {
  if (fileName.includes('@ohos.util.stream.d.ts') && value.className === 'Transform') {
    return true;
  }
  return false;
}

/**
 * judgment ModuleEntity
 * @param props
 * @returns
 */
function judgmentModuleEntity(props: JudgmentModuleEntityProps): JudgmentModuleEntityBack {
  if (props.moduleEntity.typeAliasDeclarations.length > 0) {
    props.moduleEntity.typeAliasDeclarations.forEach(value => {
      props.outBody +=
        generateTypeAliasDeclaration(value, true, props.sourceFile, props.extraImport, props.mockApi) + '\n';
    });
  }
  if (props.moduleEntity.moduleImportEquaqls.length > 0) {
    props.moduleEntity.moduleImportEquaqls.forEach(value => {
      props.outBody += generateImportEqual(value) + '\n';
    });
  }
  if (props.moduleEntity.classDeclarations.length > 0) {
    props.outBody = generateClassDeclarations(props);
  }
  if (props.moduleEntity.interfaceDeclarations.length > 0) {
    props.moduleEntity.interfaceDeclarations.forEach(value => {
      props.outBody += generateInterfaceDeclaration(value, props.sourceFile, false, props.mockApi,
        props.moduleEntity.interfaceDeclarations, props.importDeclarations, props.extraImport) + ';\n';
    });
  }
  if (props.moduleEntity.enumDeclarations.length > 0) {
    props.moduleEntity.enumDeclarations.forEach(value => {
      props.enumBody += generateEnumDeclaration(props.moduleName, value) + '\n';
    });
  }
  return {
    outBody: props.outBody,
    moduleBody: props.moduleBody,
    enumBody: props.enumBody
  };
}

/**
 * generate classDeclarations
 * @param props
 * @returns
 */
function generateClassDeclarations(props: JudgmentModuleEntityProps): string {
  let extraOutBody = '';
  props.moduleEntity.classDeclarations.forEach(value => {
    const body =
      generateClassDeclaration(props.moduleName, value, false, '', '', props.sourceFile, false, props.mockApi) + '\n';
    if (handleExtraClassDeclarationBody(value, props.sourceFile.fileName)) {
      extraOutBody = body;
    } else {
      props.outBody += body;
    }
  });
  props.outBody += extraOutBody;
  return props.outBody;
}

/**
 * defaultExportClass ForEach
 * @param props
 * @returns
 */
function defaultExportClassForEach(props: DefaultExportClassProps): DefaultExportClassBack {
  const defaultExportClass = getDefaultExportClassDeclaration(props.sourceFile);

  if (defaultExportClass.length > 0) {
    defaultExportClass.forEach(value => {
      if (
        value.exportModifiers.includes(SyntaxKind.DefaultKeyword) &&
        value.exportModifiers.includes(SyntaxKind.ExportKeyword)
      ) {
        const moduleBodyAndOutBodyBack = getModuleBodyAndOutBody(props, value);
        props.outBody = moduleBodyAndOutBodyBack.outBody;
        props.moduleBody = moduleBodyAndOutBodyBack.moduleBody;
      }
    });
  }
  return {
    outBody: props.outBody,
    moduleBody: props.moduleBody
  };
}

/**
 * get ModuleBodyAndOutBody
 * @param props
 * @param value
 * @returns
 */
function getModuleBodyAndOutBody(props: DefaultExportClassProps, value: ClassEntity): DefaultExportClassBack {
  if (props.filename.startsWith('system_')) {
    const mockNameArr = props.filename.split('_');
    const mockName = mockNameArr[mockNameArr.length - 1];
    addToSystemIndexArray({
      filename: props.filename,
      mockFunctionName: `mock${firstCharacterToUppercase(mockName)}`
    });

    props.moduleBody += `global.systemplugin.${mockName} = {`;
    if (value.staticMethods.length > 0) {
      let staticMethodBody = '';
      value.staticMethods.forEach(val => {
        staticMethodBody += generateStaticFunction(val, true, props.sourceFile, props.mockApi) + '\n';
      });
      props.moduleBody += staticMethodBody;
    }
    props.moduleBody += '}';
  } else {
    props.outBody += generateClassDeclaration(
      '',
      value,
      false,
      '',
      props.filename,
      props.sourceFile,
      false,
      props.mockApi
    );
  }
  return {
    outBody: props.outBody,
    moduleBody: props.moduleBody
  };
}

function generateInnerModuleDeclaration(
  moduleEntity: ModuleBlockEntity,
  sourceFile: SourceFile,
  filename: string,
  mockApi: string,
  extraImport: string[],
  importDeclarations: ImportElementEntity[]
): string {
  const innerModuleBody = '';
  let innerModuleName = moduleEntity.moduleName.replace(/["']/g, '');
  let moduleBody = `function mock${innerModuleName}() {\n`;
  let innerOutBody = '';
  const innerFunctionBody = '';
  const moduleEntityLoopBack = moduleEntityLoop({
    moduleEntity,
    innerOutBody,
    moduleBody,
    sourceFile,
    mockApi,
    extraImport,
    innerModuleName,
    importDeclarations
  });
  innerOutBody = moduleEntityLoopBack.innerOutBody;
  moduleBody = moduleEntityLoopBack.moduleBody;
  const moduleEntityNextBack = moduleEntityNext({
    moduleEntity,
    innerFunctionBody,
    innerModuleBody,
    filename,
    moduleBody,
    sourceFile,
    mockApi,
    extraImport,
    innerModuleName,
    importDeclarations
  });
  innerModuleName = moduleEntityNextBack.innerModuleName;
  moduleBody = moduleEntityNextBack.moduleBody;
  moduleBody += '\t};';
  moduleBody += `\n\treturn ${innerModuleName};}\n`;
  moduleBody += innerOutBody;
  return moduleBody;
}

/**
 * moduleEntity judgment
 * @param props
 * @returns
 */
function moduleEntityLoop(props: ModuleEntityLoopProps): ModuleEntityLoopBack {
  if (props.moduleEntity.typeAliasDeclarations.length) {
    props.moduleEntity.typeAliasDeclarations.forEach(value => {
      props.innerOutBody +=
        generateTypeAliasDeclaration(value, true, props.sourceFile, props.extraImport, props.mockApi) + '\n';
    });
  }
  if (props.moduleEntity.moduleImportEquaqls.length) {
    props.moduleEntity.moduleImportEquaqls.forEach(value => {
      props.innerOutBody += generateImportEqual(value) + '\n';
    });
  }

  if (props.moduleEntity.classDeclarations.length) {
    props.moduleEntity.classDeclarations.forEach(value => {
      if (value.exportModifiers.length && value.exportModifiers.includes(SyntaxKind.ExportKeyword)) {
        props.innerOutBody += generateClassDeclaration(props.innerModuleName, value, false, '', '', props.sourceFile, false, props.mockApi) + '\n';
      } else {
        props.moduleBody += '\t' + generateClassDeclaration(props.innerModuleName, value, false, '', '', props.sourceFile, true, props.mockApi) + '\n';
      }
    });
  }
  if (props.moduleEntity.interfaceDeclarations.length) {
    props.moduleEntity.interfaceDeclarations.forEach(value => {
      if (value.exportModifiers.length) {
        props.innerOutBody += generateInterfaceDeclaration(value, props.sourceFile, false, props.mockApi,
          props.moduleEntity.interfaceDeclarations, props.importDeclarations, props.extraImport) + ';\n';
      } else {
        props.moduleBody += '\t' + generateInterfaceDeclaration(value, props.sourceFile, false, props.mockApi,
          props.moduleEntity.interfaceDeclarations, props.importDeclarations, props.extraImport) + ';\n';
      }
    });
  }
  if (props.moduleEntity.enumDeclarations.length) {
    props.moduleEntity.enumDeclarations.forEach(value => {
      if (value.exportModifiers.length) {
        props.innerOutBody += generateEnumDeclaration(props.innerModuleName, value) + '\n';
      } else {
        props.moduleBody += generateEnumDeclaration(props.innerModuleName, value);
      }
    });
  }
  return {
    moduleBody: props.moduleBody,
    innerOutBody: props.innerOutBody
  };
}

/**
 * Next moduleEntity judgment
 * @param props
 * @returns
 */
function moduleEntityNext(props: ModuleEntityNextProps): ModuleEntityNextBack {
  if (props.moduleEntity.functionDeclarations.size) {
    props.moduleEntity.functionDeclarations.forEach(value => {
      props.innerFunctionBody += '\n' + generateCommonFunction(props.innerModuleName, value,
        props.sourceFile, props.mockApi, false) + '\n';
    });
  }

  if (props.moduleEntity.moduleDeclarations.length) {
    props.moduleEntity.moduleDeclarations.forEach(value => {
      if (!value.moduleName.startsWith("'") && !value.moduleName.startsWith('"')) {
        props.innerModuleBody += generateInnerModuleDeclaration(value, props.sourceFile, props.filename,
          props.mockApi, props.extraImport, props.importDeclarations);
      }
    });
  }
  if (props.innerModuleBody) {
    props.moduleBody += props.innerModuleBody + '\n';
  }

  props.moduleBody += `const ${props.innerModuleName} = {\n`;
  if (props.moduleEntity.variableStatements.length) {
    props.moduleEntity.variableStatements.forEach(value => {
      value.forEach(val => {
        props.moduleBody += generateVariableStatementDelcatation(val, false) + '\n';
      });
    });
  }

  props.moduleBody += props.innerFunctionBody + '\n';

  const exportArr = getModuleExportElements(props.moduleEntity);
  let innerExportString = '';
  exportArr.forEach(value => {
    if (value.type === 'module' && !value.name.startsWith("'") && !value.name.startsWith('"')) {
      innerExportString += `${value.name}: mock${value.name}(),\n`;
    } else {
      innerExportString += `${value.name}: ${value.name},\n`;
    }
  });
  if (innerExportString !== '') {
    props.moduleBody += '\t' + innerExportString;
  }
  return {
    innerModuleName: props.innerModuleName,
    moduleBody: props.moduleBody
  };
}

/**
 * generate inner module for declare module
 * @param moduleEntity
 * @returns
 */
function generateInnerDeclareModule(moduleEntity: ModuleBlockEntity): string {
  const moduleName = '$' + moduleEntity.moduleName.replace(/["']/g, '');
  let module = `\n\texport const ${moduleName} = `;
  if (moduleEntity.exportDeclarations.length > 0) {
    moduleEntity.exportDeclarations.forEach(value => {
      module += value.match(/{[^{}]*}/g)[0] + '\n';
    });
  }
  return module;
}

/**
 * generate inner module
 * @param moduleEntity
 * @param sourceFile
 * @param extraImport
 * @returns
 */
function generateInnerModule(
  moduleEntity: ModuleBlockEntity,
  sourceFile: SourceFile,
  extraImport: string[],
  mockApi: string
): string {
  const moduleName = moduleEntity.moduleName;
  let innerModuleBody = `const ${moduleName} = (()=> {`;

  if (moduleEntity.enumDeclarations.length > 0) {
    moduleEntity.enumDeclarations.forEach(value => {
      innerModuleBody += generateEnumDeclaration(moduleName, value) + '\n';
    });
  }

  if (moduleEntity.typeAliasDeclarations.length > 0) {
    moduleEntity.typeAliasDeclarations.forEach(value => {
      innerModuleBody += generateTypeAliasDeclaration(value, true, sourceFile, extraImport, mockApi) + '\n';
    });
  }

  if (moduleEntity.moduleImportEquaqls.length > 0) {
    moduleEntity.moduleImportEquaqls.forEach(value => {
      innerModuleBody += generateImportEqual(value) + '\n';
    });
  }

  if (moduleEntity.interfaceDeclarations.length > 0) {
    moduleEntity.interfaceDeclarations.forEach(value => {
      innerModuleBody += generateInterfaceDeclaration(value, sourceFile, false, '', moduleEntity.interfaceDeclarations) + '\n';
    });
  }

  let functionBody = 'return {';
  if (moduleEntity.functionDeclarations.size > 0) {
    moduleEntity.functionDeclarations.forEach(value => {
      functionBody += generateCommonFunction(moduleName, value, sourceFile, '', false) + '\n';
    });
  }

  if (moduleEntity.variableStatements.length > 0) {
    moduleEntity.variableStatements.forEach(value => {
      value.forEach(val => {
        innerModuleBody += generateVariableStatementDelcatation(val, true) + '\n';
      });
    });
  }
  innerModuleBody += functionBody + '\n';

  const exports = getModuleExportElements(moduleEntity);
  let exportString = '';
  exports.forEach(value => {
    exportString += `${value.name}: ${value.name},\n`;
  });
  if (exportString !== '') {
    innerModuleBody += '\t' + exportString;
  }
  innerModuleBody += '\t};})();';
  return innerModuleBody;
}

/**
 * get all export elements
 * @param moduleEntity
 * @returns
 */
function getModuleExportElements(moduleEntity: ModuleBlockEntity): Array<ModuleExportEntity> {
  const exportElements: Array<ModuleExportEntity> = [];
  if (moduleEntity.moduleName.startsWith('"') && moduleEntity.moduleName.endsWith('"')) {
    return exportElements;
  }
  if (moduleEntity.classDeclarations.length > 0) {
    moduleEntity.classDeclarations.forEach(value => {
      exportElements.push({ name: firstCharacterToUppercase(value.className), type: 'class' });
    });
  }

  if (moduleEntity.interfaceDeclarations.length > 0) {
    moduleEntity.interfaceDeclarations.forEach(value => {
      exportElements.push({ name: value.interfaceName, type: 'interface' });
    });
  }

  if (moduleEntity.enumDeclarations.length > 0) {
    moduleEntity.enumDeclarations.forEach(value => {
      exportElements.push({ name: value.enumName, type: 'enum' });
    });
  }

  if (moduleEntity.moduleDeclarations.length > 0) {
    moduleEntity.moduleDeclarations.forEach(value => {
      exportElements.push({ name: value.moduleName, type: 'module' });
    });
  }

  if (moduleEntity.typeAliasDeclarations.length > 0) {
    moduleEntity.typeAliasDeclarations.forEach(value => {
      exportElements.push({ name: value.typeAliasName, type: 'type' });
    });
  }
  return exportElements;
}
