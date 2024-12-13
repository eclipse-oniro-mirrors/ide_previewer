/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import ts, { SyntaxKind } from 'typescript';
import { KeyValue, Members, MockBuffer } from '../types';
import { BASE_KINDS, COMPONENT_DECORATORS, D_ETS, DECLARES, KeyValueTypes } from './constants';
import { associateTypeParameters, generateKeyValue, getAbsolutePath } from './commonUtils';
import path from 'path';

/**
 * 处理import导入节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 */
export function handleImportDeclaration(
  node: ts.ImportDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): void {
  const moduleSpecifier = handleExpression(node.moduleSpecifier, mockBuffer, {}, parent, type, []);
  const importFilePath = getAbsolutePath(mockBuffer, moduleSpecifier.key);
  const importClauseNode = node.importClause;
  if (importClauseNode) {
    handleImportClauseNode(importClauseNode, mockBuffer, members, parent, KeyValueTypes.IMPORT, importFilePath);
  }
}

/**
 * 处理module节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleModuleDeclaration(
  node: ts.ModuleDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const moduleName = handleModuleName(node.name, members, parent, type);
  moduleName.isNeedMock = true;
  if (!moduleName) {
    return;
  }
  handleDefaultOrExport(mockBuffer, moduleName, node.modifiers);

  handleModuleBody(mockBuffer, moduleName.members, moduleName, KeyValueTypes.PROPERTY, node.body);
  return moduleName;
}

/**
 * 处理type节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleTypeAliasDeclaration(
  node: ts.TypeAliasDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const name = handleIdentifier(node.name, members, parent, type);
  node.typeParameters?.forEach(typeParam => {
    handleTypeParameterDeclaration(typeParam, mockBuffer, name.typeParameters, name, KeyValueTypes.REFERENCE);
  });
  handleTypeNode(mockBuffer, name.members, name, KeyValueTypes.REFERENCE, node.type);
  handleDefaultOrExport(mockBuffer, name, node.modifiers);
  return name;
}

/**
 * 处理class节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleClassDeclaration(
  node: ts.ClassDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const isComponent: boolean = isComponentNode(node, mockBuffer);
  const className = handleIdentifier(node.name, members, parent, type);
  className.isNeedMock = true;
  handleDefaultOrExport(mockBuffer, className, node.modifiers);
  if (isComponent) {
    return className;
  }

  node.typeParameters?.forEach(typeParameterNode => {
    handleTypeParameterDeclaration(typeParameterNode, mockBuffer, className.typeParameters, className, KeyValueTypes.REFERENCE);
  });

  if (!className.key) {
    throw new Error('ClassDeclaration 没有名字');
  }
  node.heritageClauses?.forEach(heritageClause => {
    className.heritage = handleHeritageClause(heritageClause, mockBuffer, {}, className, KeyValueTypes.REFERENCE);
  });

  node.members.forEach(member => {
    handleClassElement(member, mockBuffer, className.members, className, KeyValueTypes.PROPERTY);
  });
  return className;
}

/**
 * 处理interface节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleInterfaceDeclaration(
  node: ts.InterfaceDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const interfaceName = handleIdentifier(node.name, members, parent, type);
  handleDefaultOrExport(mockBuffer, interfaceName, node.modifiers);
  node.typeParameters?.forEach(typeParameter => {
    handleTypeParameterDeclaration(typeParameter, mockBuffer, interfaceName.typeParameters, interfaceName, KeyValueTypes.REFERENCE);
  });
  node.heritageClauses?.forEach(heritageClause => {
    interfaceName.heritage = handleHeritageClause(heritageClause, mockBuffer, {}, interfaceName, KeyValueTypes.REFERENCE);
  });
  node.members.forEach(member => {
    handleTypeElement(member, mockBuffer, interfaceName.members, interfaceName, KeyValueTypes.PROPERTY);
  });
  return interfaceName;
}

/**
 * 处理enum节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleEnumDeclaration(
  node: ts.EnumDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const enumName = handleEnumName(node, members, parent, type);
  enumName.isNeedMock = true;
  handleDefaultOrExport(mockBuffer, enumName, node.modifiers);
  node.members.forEach(member => {
    handleEnumMember(member, mockBuffer, enumName.members, enumName, KeyValueTypes.PROPERTY);
  });
  return enumName;
}

/**
 * 处理function节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleFunctionDeclaration(
  node: ts.FunctionDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const functionName = handleIdentifier(node.name, members, parent, type);
  functionName.isNeedMock = true;
  handleDefaultOrExport(mockBuffer, functionName, node.modifiers);
  for (let i = 0; i < node.parameters.length; i++) {
    handleParameterDeclaration(i, node.parameters[i], mockBuffer, functionName.methodParams, functionName, KeyValueTypes.REFERENCE);
  }
  node.typeParameters?.forEach(
    typeParameter => handleTypeParameterDeclaration(typeParameter, mockBuffer, functionName.typeParameters, functionName, KeyValueTypes.VALUE)
  );
  handleTypeNode(mockBuffer, functionName.members, functionName, KeyValueTypes.REFERENCE, node.type);
  return functionName;
}

/**
 * 处理export节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleExportDeclaration(
  node: ts.ExportDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): void {
  let moduleSpecifier: KeyValue;
  let importedModulePath: string;
  if (node.moduleSpecifier) {
    moduleSpecifier = handleExpression(node.moduleSpecifier, mockBuffer, {}, parent, KeyValueTypes.EXPORT, []);
    importedModulePath = getAbsolutePath(mockBuffer, moduleSpecifier.key);
    moduleSpecifier.key = importedModulePath;
  }

  if (node.exportClause) {
    if (importedModulePath) {
      handleNamedExportBindings(node.exportClause, mockBuffer, members, parent, type, importedModulePath);
    }
  } else {
    if (moduleSpecifier) {
      members[moduleSpecifier.key] = moduleSpecifier;
    } else {
      throw new Error('当导出*时，未指定模块路径');
    }
  }
}

/**
 * 处理const节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleVariableStatement(
  node: ts.VariableStatement,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): void {
  node.declarationList.declarations.forEach(declaration => {
    const declarationName = handleBindingName(declaration.name, mockBuffer, members, parent, type

    );
    declarationName.isNeedMock = true;
    declarationName.operateElements = [];
    handleDefaultOrExport(mockBuffer, declarationName, node.modifiers);
    declaration.initializer && handleExpression(declaration.initializer, mockBuffer, declarationName.members, declarationName, KeyValueTypes.REFERENCE, declarationName.operateElements);
    declaration.type && handleTypeNode(mockBuffer, declarationName.members, declarationName, KeyValueTypes.REFERENCE, declaration.type);
  });
}

/**
 * 处理export节点
 * @param node typescript node节点
 * @param mockBuffer mock信息
 * @param members 当前节点应该归属的member
 * @param parent 父级节点
 * @param type KeyValue节点类型
 * @returns
 */
export function handleExportAssignment(
  node: ts.ExportAssignment,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const exportDefault = handleExpression(node.expression, mockBuffer, {}, parent, type, []);
  members[exportDefault.key].isDefault = true;
  mockBuffer.contents.members.default = members[exportDefault.key];
  return exportDefault;
}

function handleNamedExportBindings(
  node: ts.NamedExportBindings,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  importedModulePath: string
): void {
  if (!node) {
    throw new Error('NamedExportBindings类型node为undefined');
  }
  switch (node.kind) {
    case SyntaxKind.NamedExports: {
      return handleNamedExports(node, mockBuffer, members, parent, type, importedModulePath);
    }
    case SyntaxKind.NamespaceExport: {
      break;
    }
    default: {
      throw new Error('未知的NamedExportBindings类型');
    }
  }
}

function handleNamedExports(
  node: ts.NamedExports,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  importedModulePath: string
): void {
  node.elements.forEach(element => {
    if (importedModulePath) {
      const exportName = handleIdentifier(element.name, mockBuffer.contents.members, mockBuffer.contents, type);
      exportName.importedModulePath = importedModulePath;
    }
    handleIdentifier(element.name, mockBuffer.contents.members, mockBuffer.contents, KeyValueTypes.REFERENCE);
  });
}

function handleParameterDeclaration(
  parameterNum: number,
  node: ts.ParameterDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const name = handleBindingName(node.name, mockBuffer, members, parent, type);
  if (name.key === 'arguments') {
    delete members.arguments;
    name.key = name.key.replace('arguments', 'args');
    members['args'] = name;
  }
  handleTypeNode(mockBuffer, name.members, name, KeyValueTypes.REFERENCE, node.type);
  return name;
}

function handleBindingName(
  node: ts.BindingName,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  let text: string;
  switch (node.kind) {
    case SyntaxKind.Identifier: {
      text = node.escapedText.toString();
      break;
    }
    case SyntaxKind.ObjectBindingPattern: {
      return handleObjectBindingPattern();
    }
    default: {
      throw new Error('未知的Statement类型');
    }
  }
  members[text] = generateKeyValue(text, type, parent);
  return members[text];
}

function handleObjectBindingPattern(): KeyValue {
  return generateKeyValue('', KeyValueTypes.VALUE);
}

function handleEnumName(
  node: ts.EnumDeclaration,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const enumName = node.name.escapedText.toString();
  members[enumName] = generateKeyValue(enumName, type, parent);
  return members[enumName];
}

function handleEnumMember(
  node: ts.EnumMember,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const memberName = handlePropertyNameNode(node.name, mockBuffer, members, parent, type);
  if (node.initializer) {
    handleEnumMemberInitializer(node.initializer, mockBuffer, memberName.members, memberName, KeyValueTypes.REFERENCE);
  } else {
    memberName.value = '0';
  }
  return memberName;
}

function handleEnumMemberInitializer(
  node: ts.Expression,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (!node) {
    return generateKeyValue('', KeyValueTypes.VALUE, parent);
  }

  switch (node.kind) {
    case SyntaxKind.NumericLiteral: {
      const numericLiteral = node as ts.NumericLiteral;
      members[numericLiteral.text] = generateKeyValue(numericLiteral.text, KeyValueTypes.VALUE, parent);
      return members[numericLiteral.text];
    }
    case SyntaxKind.StringLiteral: {
      return handleStringLiteral(node as ts.StringLiteral, members, parent);
    }
    case SyntaxKind.BinaryExpression: {
      return handleBinaryExpression(node as ts.BinaryExpression, mockBuffer, members, parent);
    }
    case SyntaxKind.PrefixUnaryExpression: {
      return handlePrefixUnaryExpression(node as ts.PrefixUnaryExpression, members, parent);
    }
    case SyntaxKind.Identifier: {
      return handleIdentifier(node as ts.Identifier, members, parent, type);
    }
    case SyntaxKind.PropertyAccessExpression: {
      const expression = generateKeyValue('expression', KeyValueTypes.EXPRESSION, parent);
      expression.operateElements = [];
      members['expression'] = expression;
      return handlePropertyAccessExpression(
          node as ts.PropertyAccessExpression,
          mockBuffer, expression.members,
          expression.parent,
          type,
          expression.operateElements
      );
    }
    default: {
      throw new Error('未知的EnumMemberInitializer类型');
    }
  }
}

function handleTypeElement(
  node: ts.TypeElement,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  switch (node.kind) {
    case SyntaxKind.PropertySignature: {
      return handlePropertySignature(node as ts.PropertySignature, mockBuffer, members, parent, type);
    }
    case SyntaxKind.MethodSignature: {
      return handleMethodSignature(node as ts.MethodSignature, mockBuffer, members, parent, KeyValueTypes.FUNCTION);
    }
    case SyntaxKind.IndexSignature: {
      return handleIndexSignature();
    }
    case SyntaxKind.CallSignature: {
      return handleCallSignatureDeclaration(node as ts.CallSignatureDeclaration, mockBuffer, members, parent, type);
    }
    case SyntaxKind.ConstructSignature: {
      return generateKeyValue('', type, parent);
    }
    default: {
      throw new Error('未知的TypeElement类型');
    }
  }
}

function handleCallSignatureDeclaration(
  node: ts.CallSignatureDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (node.questionToken) {
    return;
  }
  return handleMethodSignature(node, mockBuffer, members, parent, type);
}

function handleIndexSignature(): KeyValue {
  return generateKeyValue('', KeyValueTypes.VALUE);
}

function handleMethodSignature(
  node: ts.CallSignatureDeclaration | ts.MethodSignature,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (node.questionToken) {
    return;
  }
  if (!node.name) {
    return;
  }
  const methodName = handlePropertyNameNode(node.name, mockBuffer, members, parent, type);
  for (let i = 0; i < node.parameters.length; i++) {
    node.parameters.forEach(_ => handleParameterDeclaration(
      i,
      node.parameters[i],
      mockBuffer,
      methodName.methodParams,
      methodName,
      KeyValueTypes.REFERENCE
    ));
  }
  node.typeParameters?.forEach(parameter => {
    handleTypeParameterDeclaration(parameter, mockBuffer, methodName.typeParameters, methodName, KeyValueTypes.VALUE);
  });
  handleTypeNode(mockBuffer, methodName.members, methodName, KeyValueTypes.REFERENCE, node.type);
  return methodName;
}

function handlePropertySignature(
  node: ts.PropertySignature,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (node.questionToken) {
    return;
  }
  const propertyName = handlePropertyNameNode(node.name, mockBuffer, members, parent, type);
  return handleTypeNode(mockBuffer, propertyName.members, propertyName, KeyValueTypes.REFERENCE, node.type);
}

function handleClassElement(
  node: ts.ClassElement,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): void {
  switch (node.kind) {
    case SyntaxKind.PropertyDeclaration: {
      handlePropertyDeclaration(node as ts.PropertyDeclaration, mockBuffer, members, parent, type);
      return;
    }
    case SyntaxKind.MethodDeclaration: {
      handleMethodDeclaration(node as ts.MethodDeclaration, mockBuffer, members, parent, KeyValueTypes.FUNCTION);
      return;
    }
    case SyntaxKind.IndexSignature:
    case SyntaxKind.Constructor: {
      return;
    }
    case SyntaxKind.GetAccessor: {
      handleGetAccessorDeclaration(node as ts.GetAccessorDeclaration, mockBuffer, members, parent, KeyValueTypes.FUNCTION);
      return;
    }
    case SyntaxKind.SetAccessor: {
      handleSetAccessorDeclaration(node as ts.SetAccessorDeclaration, mockBuffer, members, parent, KeyValueTypes.FUNCTION);
      return;
    }
    default: {
      throw new Error('未知的ClassElement类型');
    }
  }
}

function handleMethodDeclaration(
  node: ts.MethodDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (node.questionToken) {
    return;
  }
  const methodName = handlePropertyNameNode(node.name, mockBuffer, members, parent, type);
  methodName.isStatic = node.modifiers?.some(modify => modify.kind === SyntaxKind.StaticKeyword);
  node.typeParameters?.forEach(typeParameter => {
    handleTypeParameterDeclaration(typeParameter, mockBuffer, methodName.typeParameters, methodName, KeyValueTypes.REFERENCE);
  });
  for (let i = 0; i < node.parameters.length; i++) {
    handleParameterDeclaration(i, node.parameters[i], mockBuffer, methodName.methodParams, methodName, KeyValueTypes.REFERENCE);
  }
  handleTypeNode(mockBuffer, methodName.members, methodName, KeyValueTypes.REFERENCE, node.type);
  return methodName;
}

function handleHeritageClause(
  node: ts.HeritageClause,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (node.token === SyntaxKind.ImplementsKeyword) {
    return;
  }
  return handleExpressionWithTypeArguments(node.types[node.types.length - 1], mockBuffer, members, parent, type, []);
}

function handleExpressionWithTypeArguments(
  node: ts.ExpressionWithTypeArguments,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  operateElements: KeyValue[]
): KeyValue {
  const expression = handleLeftHandSideExpression(node.expression, mockBuffer, members, parent, type, operateElements);
  node.typeArguments?.forEach(typeNode => {
    handleTypeNode(mockBuffer, expression.typeParameters, expression, KeyValueTypes.REFERENCE, typeNode);
  });
  return expression;
}

function handleLeftHandSideExpression(
  node: ts.LeftHandSideExpression,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  operateElements: KeyValue[]
): KeyValue {
  switch (node.kind) {
    case SyntaxKind.Identifier: {
      const keyValue = handleIdentifier(node as ts.Identifier, members, parent, KeyValueTypes.REFERENCE);
      operateElements.push(keyValue);
      return keyValue;
    }
    case SyntaxKind.PropertyAccessExpression: {
      return handlePropertyAccessExpression(node as ts.PropertyAccessExpression, mockBuffer, members, parent, type, operateElements);
    }
    default: {
      throw new Error('未知的LeftHandSideExpression类型');
    }
  }
}

function isComponentNode(node: ts.ClassDeclaration, mockBuffer: MockBuffer): boolean {
  if (!mockBuffer.rawFilePath.endsWith(D_ETS)) {
    return false;
  }
  if (!node.decorators) {
    return false;
  }
  return !!node.decorators.find(decorator => {
    return COMPONENT_DECORATORS.includes(
      handleExpression(decorator.expression, mockBuffer, {}, {} as KeyValue, KeyValueTypes.REFERENCE, []).key
    );
  });
}

function handleImportClauseNode(
  node: ts.ImportClause,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  importFilePath: string
): void {
  const namedImportBindings = node.namedBindings;
  if (namedImportBindings) {
    handleNamedBindings(namedImportBindings, members, parent, type, importFilePath);
  }

  if (node.name) {
    const defaultExportName = handleIdentifier(node.name, members, parent, KeyValueTypes.IMPORT);
    defaultExportName.isImportDefault = true;
    defaultExportName.importedModulePath = importFilePath;
  }
}

function handlePropertyNameNode(
  node: ts.PropertyName,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (!node) {
    members[''] = generateKeyValue('', type, parent);
    return members[''];
  }
  switch (node.kind) {
    case SyntaxKind.Identifier: {
      return handleIdentifier(node, members, parent, type);
    }
    case SyntaxKind.ComputedPropertyName: {
      return handleComputedPropertyName(node, mockBuffer, members, parent, type);
    }
    default: {
      throw new Error('位置的PropertyNameNode类型');
    }
  }
}

function handleComputedPropertyName(
  node: ts.ComputedPropertyName,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const expression = generateKeyValue('expression', KeyValueTypes.EXPRESSION, parent);
  expression.operateElements = [];
  let randomName = `expression_${Math.random()}`;
  while (members[randomName]) {
    randomName = `expression_${Math.random()}`;
  }
  members[randomName] = expression;

  handleExpression(node.expression, mockBuffer, {}, expression, type, expression.operateElements);
  return expression;
}

function handleNamedBindings(
  node: ts.NamedImportBindings,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  importFilePath: string
): void {
  switch (node.kind) {
    case SyntaxKind.NamespaceImport: {
      handleNamespaceImport(node, members, parent, type, importFilePath);
      break;
    }
    case SyntaxKind.NamedImports: {
      node.elements.forEach(element => {
        handleNamedImports(element, members, parent, type, importFilePath);
      });
      break;
    }
  }
}

function handleNamespaceImport(
  node: ts.NamespaceImport,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  importFilePath: string
): KeyValue {
  const namespaceName = handleIdentifier(node.name, members, parent, type);
  namespaceName.isNamespaceImport = true;
  namespaceName.importedModulePath = importFilePath;
  return namespaceName;
}

function handleNamedImports(
  node: ts.ImportSpecifier,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  importFilePath: string
): KeyValue {
  const name = handleIdentifier(node.name, members, parent, type);
  if (node.propertyName) {
    name.rawName = handleIdentifier(node.propertyName, {}, parent, type).key;
  }

  name.importedModulePath = importFilePath;
  return name;
}

function handleDefaultOrExport(
  mockBuffer: MockBuffer,
  keyValue: KeyValue,
  modifiers?: ts.ModifiersArray
): void {
  if (!modifiers) {
    return;
  }
  if (modifiers.findIndex(modifier => modifier.kind === SyntaxKind.DefaultKeyword) >= 0) {
    mockBuffer.contents.members['default'] = keyValue;
    keyValue.isDefault = true;
  }

  if (modifiers.findIndex(modifier => modifier.kind === SyntaxKind.DeclareKeyword) >= 0) {
    if (!DECLARES[keyValue.key]) {
      DECLARES[keyValue.key] = { keyValue, from: mockBuffer.mockedFilePath };
    } else {
      DECLARES[keyValue.key].keyValue.sameDeclares.push({keyValue, from: mockBuffer.mockedFilePath});
    }
    keyValue.isGlobalDeclare = true;
  }
}

function handleModuleName(
  node: ts.ModuleName,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  switch (node.kind) {
    case SyntaxKind.Identifier: {
      return handleIdentifier(node, members, parent, type);
    }
    case SyntaxKind.StringLiteral: {
      members[''] = generateKeyValue('', type, parent);
      return members[''];
    }
    default: {
      throw new Error('未知的ModuleName类型');
    }
  }
}

function handleModuleBody(
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  node?: ts.ModuleBody | ts.JSDocNamespaceDeclaration
): void {
  switch (node.kind) {
    case SyntaxKind.ModuleBlock: {
      node.statements.forEach(statement => {
        handleStatement(statement, mockBuffer, members, parent);
      });
      return;
    }
    default: {
      throw new Error('未知的ModuleBody类型');
    }
  }
}

function handleStatement(
  node: ts.Statement,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue
): void {
  switch (node.kind) {
    case SyntaxKind.EnumDeclaration: {
      handleEnumDeclaration(node as ts.EnumDeclaration, mockBuffer, members, parent, KeyValueTypes.ENUM);
      break;
    }
    case SyntaxKind.InterfaceDeclaration: {
      handleInterfaceDeclaration(node as ts.InterfaceDeclaration, mockBuffer, members, parent, KeyValueTypes.INTERFACE);
      break;
    }
    case SyntaxKind.VariableStatement: {
      handleVariableStatement(node as ts.VariableStatement, mockBuffer, members, parent, KeyValueTypes.VARIABLE);
      break;
    }
    case SyntaxKind.FunctionDeclaration: {
      handleFunctionDeclaration(node as ts.FunctionDeclaration, mockBuffer, members, parent, KeyValueTypes.FUNCTION);
      break;
    }
    case SyntaxKind.ClassDeclaration: {
      handleClassDeclaration(node as ts.ClassDeclaration, mockBuffer, members, parent, KeyValueTypes.CLASS);
      break;
    }
    case SyntaxKind.TypeAliasDeclaration: {
      handleTypeAliasDeclaration(node as ts.TypeAliasDeclaration, mockBuffer, members, parent, KeyValueTypes.VARIABLE);
      break;
    }
    case SyntaxKind.ExportDeclaration: {
      handleExportDeclaration(node as ts.ExportDeclaration, mockBuffer, members, parent, KeyValueTypes.IMPORT);
      break;
    }
    case SyntaxKind.ModuleDeclaration: {
      handleModuleDeclaration(node as ts.ModuleDeclaration, mockBuffer, members, parent, KeyValueTypes.MODULE);
      break;
    }
    case SyntaxKind.ImportEqualsDeclaration: {
      handleImportEqualsDeclaration(node as ts.ImportEqualsDeclaration, mockBuffer, members, parent, KeyValueTypes.VARIABLE);
      break;
    }
    default: {
      throw new Error('未知的Statement类型');
    }
  }
}

function handleImportEqualsDeclaration(
  node: ts.ImportEqualsDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const name = handleIdentifier(node.name, members, parent, type);
  handleModuleReference(node.moduleReference, mockBuffer, name.members, name, type);
  return name;
}

function handleModuleReference(
  node: ts.ModuleReference,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  switch (node.kind) {
    case SyntaxKind.QualifiedName: {
      return handleQualifiedName(node as ts.QualifiedName, mockBuffer, members, parent, type);
    }
    default: {
      throw new Error('未知的ModuleReference类型');
    }
  }
}

function handleGetAccessorDeclaration(
  node: ts.GetAccessorDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const methodName = handlePropertyNameNode(node.name, mockBuffer, {}, parent, type);
  members[`get ${methodName.key}`] = methodName;
  handleAccessorDeclaration(node, mockBuffer, methodName);
  return methodName;
}

function handleSetAccessorDeclaration(
  node: ts.SetAccessorDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const methodName = handlePropertyNameNode(node.name, mockBuffer, {}, parent, type);
  members[`set ${methodName.key}`] = methodName;
  handleAccessorDeclaration(node, mockBuffer, methodName);
  return methodName;
}

function handleAccessorDeclaration(
  node: ts.GetAccessorDeclaration | ts.SetAccessorDeclaration,
  mockBuffer: MockBuffer,
  method: KeyValue
): KeyValue {
  for (let i = 0; i < node.parameters.length; i++) {
    handleParameterDeclaration(i, node.parameters[i], mockBuffer, method.methodParams, method, KeyValueTypes.REFERENCE);
  }
  handleTypeNode(mockBuffer, method.members, method, KeyValueTypes.REFERENCE, node.type);
  return method;
}

function handleTypeParameterDeclaration(
  node: ts.TypeParameterDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const typeParameterName = handleIdentifier(node.name, members, parent, type);
  typeParameterName.value = '{name: "unknown type"}';
  handleTypeNode(mockBuffer, typeParameterName.constraint, typeParameterName, KeyValueTypes.REFERENCE, node.constraint);
  return typeParameterName;
}

function handleTypeNode(
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  node?: ts.TypeNode
): KeyValue {
  let typeText: string;
  if (!node) {
    const keyValue = generateKeyValue('\'any type\'', KeyValueTypes.VALUE, parent);
    members['\'any type\''] = keyValue;
    return keyValue;
  }
  switch (node.kind) {
    case SyntaxKind.NumberKeyword: {
      typeText = '1';
      break;
    }
    case SyntaxKind.StringKeyword: {
      typeText = 'vt';
      break;
    }
    case SyntaxKind.ArrayType: {
      typeText = '[]';
      break;
    }
    case SyntaxKind.BooleanKeyword: {
      typeText = 'true';
      break;
    }
    case SyntaxKind.TypeReference: {
      return handleTypeReferenceNode(node as ts.TypeReferenceNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.VoidKeyword: {
      const keyValue = generateKeyValue('\'undefined\'', KeyValueTypes.VALUE, parent);
      members[keyValue.key] = keyValue;
      keyValue.value = 'undefined';
      return keyValue;
    }
    case SyntaxKind.UnionType: {
      return handleUnionTypeNode(node as ts.UnionTypeNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.LiteralType: {
      return handleLiteralTypeNode(node as ts.LiteralTypeNode, mockBuffer, members, parent, KeyValueTypes.VALUE);
    }
    case SyntaxKind.TupleType: {
      return handleTupleTypeNode(node as ts.TupleTypeNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.FunctionType: {
      return handleFunctionTypeNode(node as ts.FunctionTypeNode, mockBuffer, members, parent, KeyValueTypes.FUNCTION);
    }
    case SyntaxKind.AnyKeyword:
    case SyntaxKind.ObjectKeyword:
    case SyntaxKind.TypeLiteral:
    case SyntaxKind.UnknownKeyword: {
      typeText = '{}';
      break;
    }
    case SyntaxKind.BigIntKeyword: {
      typeText = '100000000000';
      break;
    }
    case SyntaxKind.KeyOfKeyword: {
      typeText = 'keyof';
      break;
    }
    case SyntaxKind.MappedType: {
      return handleMappedTypeNode(node as ts.MappedTypeNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.TypeOperator: {
      return handleTypeOperatorNode(node as ts.TypeOperatorNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.IndexedAccessType: {
      return handleIndexedAccessTypeNode(node as ts.IndexedAccessTypeNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.ImportType: {
      return handleImportTypeNode(node as ts.ImportTypeNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.ParenthesizedType: {
      return handleParenthesizedTypeNode(node as ts.ParenthesizedTypeNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.TemplateLiteralType: {
      return handleTemplateLiteralTypeNode(node as ts.TemplateLiteralTypeNode, mockBuffer, members, parent, type);
    }
    case SyntaxKind.UndefinedKeyword: {
      return handleUndefinedKeywordNode(members, parent);
    }
    case SyntaxKind.IntersectionType: {
      return handleIntersectionTypeNode(node as ts.IntersectionTypeNode, mockBuffer, members, parent, KeyValueTypes.INTERSECTION);
    }
    case SyntaxKind.TypeQuery: {
      return handleTypeQueryNode(node as ts.TypeQueryNode, mockBuffer, members, parent, type);
    }
    default: {
      throw new Error('未知的TypeNode类型');
    }
  }
  members[typeText] = generateKeyValue(typeText, KeyValueTypes.VALUE, parent);
  return members[typeText];
}

function handleTypeQueryNode(
  node: ts.TypeQueryNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  return handleEntityName(node.exprName, mockBuffer, members, parent, type);
}

function handleEntityName(
  node: ts.EntityName,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (!node) {
    return generateKeyValue('', KeyValueTypes.VALUE, parent);
  }
  switch (node.kind) {
    case SyntaxKind.Identifier: {
      return handleIdentifier(node, members, parent, KeyValueTypes.REFERENCE);
    }
    case SyntaxKind.QualifiedName: {
      return handleQualifiedName(node as ts.QualifiedName, mockBuffer, members, parent, type);
    }
    default: {
      throw new Error('未知的EntityName类型');
    }
  }
}

function handleQualifiedName(
  node: ts.QualifiedName,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const left = handleEntityName(node.left, mockBuffer, members, parent, type);
  const right = node.right.escapedText.toString();
  left.property = generateKeyValue(right, KeyValueTypes.REFERENCE, left);
  return left;
}

function handlePropertyDeclaration(
  node: ts.PropertyDeclaration,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): void {
  if (node.questionToken) {
    return;
  }
  const propertyName = handlePropertyNameNode(node.name, mockBuffer, members, parent, type);
  if (node.initializer) {
    const expressionKeyValue = generateKeyValue('expression', KeyValueTypes.EXPRESSION, propertyName);
    propertyName.members['expression'] = expressionKeyValue;
    expressionKeyValue.operateElements = [];
    handleExpression(node.initializer, mockBuffer, {}, expressionKeyValue, KeyValueTypes.VALUE, expressionKeyValue.operateElements);
    return;
  }
  handleTypeNode(mockBuffer, propertyName.members, propertyName, KeyValueTypes.REFERENCE, node.type);
}

function handleIntersectionTypeNode(
  node: ts.IntersectionTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const intersectKeyValue = generateKeyValue('Object.assign', type, parent);
  members[intersectKeyValue.key] = intersectKeyValue;
  node.types.forEach(typeNode => handleTypeNode(mockBuffer, intersectKeyValue.methodParams, intersectKeyValue, KeyValueTypes.REFERENCE, typeNode));
  return intersectKeyValue;
}

function handleUndefinedKeywordNode(
  members: Members,
  parent: KeyValue
): KeyValue {
  members['undefined'] = generateKeyValue('undefined', KeyValueTypes.VALUE, parent);
  return members['undefined'];
}

function handleTemplateLiteralTypeNode(
  node: ts.TemplateLiteralTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const expression = generateKeyValue('expression', KeyValueTypes.EXPRESSION, parent);
  members[expression.key] = expression;
  expression.operateElements = [];

  handleTemplateHead(node.head, expression.operateElements, expression, type);

  node.templateSpans.forEach(templateSpan => handleTemplateLiteralTypeSpan(templateSpan, expression.operateElements, expression, type, mockBuffer));
  return expression;
}

function handleTemplateLiteralTypeSpan(
  node: ts.TemplateLiteralTypeSpan,
  operateElements: KeyValue[],
  parent: KeyValue,
  type: KeyValueTypes,
  mockBuffer: MockBuffer
): void {
  const addKeyValue = generateKeyValue('+', KeyValueTypes.VALUE, parent);
  addKeyValue.value = '+';

  operateElements.push(addKeyValue);
  operateElements.push(handleTypeNode(mockBuffer, {}, parent, KeyValueTypes.REFERENCE, node.type));
  operateElements.push(addKeyValue);

  switch (node.literal.kind) {
    case SyntaxKind.TemplateTail: {
      return handleTemplateTail(node.literal, operateElements, parent, KeyValueTypes.VALUE);
    }
    case SyntaxKind.TemplateMiddle: {
      return handleTemplateMiddle(node.literal, operateElements, parent, KeyValueTypes.VALUE);
    }
    default: {
      throw new Error('未知的templateSpan类型');
    }
  }
}

function handleTemplateTail(
  node: ts.TemplateTail,
  operateElements: KeyValue[],
  parent: KeyValue,
  type: KeyValueTypes
): void {
  const keyValue = generateKeyValue(node.text, type, parent);
  keyValue.value = `'${node.text}'`;
  operateElements.push(keyValue);
}

function handleTemplateMiddle(
  node: ts.TemplateMiddle,
  operateElements: KeyValue[],
  parent: KeyValue,
  type: KeyValueTypes
): void {
  const keyValue = generateKeyValue(node.text, type, parent);
  keyValue.value = `'${node.text}'`;
  operateElements.push(keyValue);
}

function handleTemplateHead(
  node: ts.TemplateHead,
  operateElements: KeyValue[],
  parent: KeyValue,
  type: KeyValueTypes
): void {
  const keyValue = generateKeyValue(node.text, type, parent);
  keyValue.value = `'${node.text}'`;
  operateElements.push(keyValue);
}

function handleParenthesizedTypeNode(
  node: ts.ParenthesizedTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  return handleTypeNode(mockBuffer, members, parent, type, node.type as ts.TypeNode);
}

function handleImportTypeNode(
  node: ts.ImportTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const argument = handleTypeNode(mockBuffer, {}, parent, type, node.argument);
  argument.type = KeyValueTypes.IMPORT;

  const typeArguments = {} as Members;
  node.typeArguments?.forEach(typeArgument => handleTypeNode(mockBuffer, typeArguments, parent, KeyValueTypes.REFERENCE, typeArgument));
  const specifier = argument.key;
  const defaultName = `imported_by_${path.basename(specifier).replace(/\./g, '_').replace(/[@'"]/g, '')}`;
  const importFilePath = getAbsolutePath(mockBuffer, specifier.replace(/['"]/g, ''));

  if (!importFilePath) {
    throw new Error(`不能到达文件 ${importFilePath}`);
  }
  const qualifier = handleEntityName(node.qualifier, mockBuffer, {}, parent, type);
  associateTypeParameters(qualifier, typeArguments);

  const moduleSpecifier = generateKeyValue(defaultName, KeyValueTypes.IMPORT, mockBuffer.contents);
  moduleSpecifier.importedModulePath = importFilePath;
  mockBuffer.contents.members[defaultName] = moduleSpecifier;
  if (qualifier.key === 'default') {
    moduleSpecifier.isImportDefault = true;
    qualifier.key = defaultName;
    members[defaultName] = qualifier;
    return qualifier;
  } else {
    moduleSpecifier.isNamespaceImport = true;
    const namespace = generateKeyValue(defaultName, KeyValueTypes.REFERENCE, parent);
    namespace.property = qualifier;
    qualifier.parent = namespace;
    members[defaultName] = namespace;
    return namespace;
  }
}

function handleIndexedAccessTypeNode(
  node: ts.IndexedAccessTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  return handleTypeNode(mockBuffer, members, parent, type, node.objectType);
}

function handleTypeOperatorNode(
  node: ts.TypeOperatorNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  return handleTypeNode(mockBuffer, members, parent, type, node.type);
}

function handleMappedTypeNode(
  node: ts.MappedTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  return handleTypeNode(mockBuffer, members, parent, type, node.type);
}

function handleFunctionTypeNode(
  node: ts.FunctionTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  let functionName: KeyValue;
  if (node.name) {
    functionName = handlePropertyNameNode(node.name, mockBuffer, members, parent, type);
  } else {
    functionName = parent;
    parent.type = KeyValueTypes.FUNCTION;
  }
  functionName.isArrowFunction = new Set<KeyValueTypes>([
    KeyValueTypes.INTERSECTION,
    KeyValueTypes.VARIABLE,
    KeyValueTypes.PROPERTY,
    KeyValueTypes.FUNCTION
  ]).has(parent.type);
  for (let i = 0; i < node.parameters.length; i++) {
    handleParameterDeclaration(i, node.parameters[i], mockBuffer, functionName.methodParams, functionName, KeyValueTypes.REFERENCE);
  }
  handleTypeNode(mockBuffer, functionName.members, functionName, KeyValueTypes.REFERENCE, node.type);
  return functionName;
}

function handleTupleTypeNode(
  node: ts.TupleTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const keyValue = generateKeyValue('Array', KeyValueTypes.REFERENCE, parent);
  parent.members['Array'] = keyValue;
  let index: number = 0;
  node.elements.forEach(element => {
    let elementKeyValue: KeyValue;
    switch (element.kind) {
      case SyntaxKind.NumberKeyword: {
        elementKeyValue = generateKeyValue('1', KeyValueTypes.VALUE, keyValue);
        break;
      }
      case SyntaxKind.StringKeyword: {
        elementKeyValue = generateKeyValue('vt', KeyValueTypes.VALUE, keyValue);
        break;
      }
      case SyntaxKind.NamedTupleMember: {
        elementKeyValue = handleNamedTupleMember(element as ts.NamedTupleMember, mockBuffer, {}, keyValue, type);
        break;
      }
      case SyntaxKind.ParenthesizedType: {
        elementKeyValue = handleParenthesizedTypeNode(element as ts.ParenthesizedTypeNode, mockBuffer, {}, keyValue, type);
        break;
      }
      case SyntaxKind.TypeReference: {
        elementKeyValue = handleTypeReferenceNode(element as ts.TypeReferenceNode, mockBuffer, {}, keyValue, type);
        break;
      }
      case SyntaxKind.OptionalType: {
        elementKeyValue = handleOptionTypeNode(element as ts.OptionalTypeNode, mockBuffer, {}, keyValue, type);
        break;
      }
      default: {
        throw new Error('未知的TupleType类型');
      }
    }
    keyValue.methodParams[index++] = elementKeyValue;
  });
  return keyValue;
}

function handleOptionTypeNode(
  node: ts.OptionalTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  return handleTypeNode(mockBuffer, members, parent, type, node.type);
}

function handleNamedTupleMember(
  node: ts.NamedTupleMember,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  return handleTypeNode(mockBuffer, members, parent, type, node.type);
}

function handleUnionTypeNode(
  node: ts.UnionTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  if (!node.types.length) {
    const undefinedKeyValue = generateKeyValue('undefined', KeyValueTypes.VALUE, parent);
    undefinedKeyValue.value = 'undefined';
    return undefinedKeyValue;
  }
  const typeIndex = node.types.findIndex(typeNode => {
    return BASE_KINDS.includes(typeNode.kind);
  });

  return handleTypeNode(mockBuffer, members, parent, type, node.types[Math.max(typeIndex, 0)]);
}

function handleLiteralTypeNode(
  node: ts.LiteralTypeNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  switch (node.literal.kind) {
    case SyntaxKind.StringLiteral: {
      return handleStringLiteral(node.literal as ts.StringLiteral, members, parent);
    }
    case SyntaxKind.TrueKeyword: {
      return handleTrueKeyword(members, parent);
    }
    case SyntaxKind.FalseKeyword: {
      return handleFalseKeyword(members, parent);
    }
    case SyntaxKind.PrefixUnaryExpression: {
      return handlePrefixUnaryExpression(node.literal as ts.PrefixUnaryExpression, members, parent);
    }
    case SyntaxKind.NumericLiteral: {
      return handleNumericLiteral(node.literal as ts.NumericLiteral, members, parent);
    }
    case SyntaxKind.NullKeyword: {
      return handleNullLiteral(members, parent, type);
    }
    case SyntaxKind.BigIntLiteral: {
      return handleBigIntLiteral(node.literal as ts.BigIntLiteral, members, parent, type);
    }
    default: {
      throw new Error('未知的literal类型');
    }
  }
}

function handleBigIntLiteral(
  node: ts.BigIntLiteral,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const literal = node.text.replace('n', '');
  members[literal] = generateKeyValue(literal, type, parent);
  return members[literal];
}

function handleNullLiteral(
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  members['null'] = generateKeyValue('null', type, parent);
  return members['null'];
}

function handlePrefixUnaryExpression(
  node: ts.PrefixUnaryExpression,
  members: Members,
  parent: KeyValue
): KeyValue {
  const expression = generateKeyValue('expression', KeyValueTypes.EXPRESSION, parent);
  members['expression'] = expression;
  expression.operateElements = [];

  const operator = handlePrefixUnaryOperator(node.operator, {}, expression, KeyValueTypes.VALUE);
  expression.operateElements.push(operator);
  expression.operateElements.push(handleUnaryExpression(node.operand, {}, operator));
  return expression;
}

function handlePrefixUnaryOperator(
  node: ts.PrefixUnaryOperator,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  let operator: string;
  switch (node) {
    case SyntaxKind.MinusToken:
      operator = '-';
      break;
    default: {
      throw new Error('未知类型的PrefixUnaryOperator');
    }
  }
  members[operator] = generateKeyValue(operator, type, parent);
  return members[operator];
}

function handleUnaryExpression(
  node: ts.UnaryExpression,
  members: Members,
  parent: KeyValue
): KeyValue {
  switch (node.kind) {
    case SyntaxKind.NumericLiteral: {
      return handleNumericLiteral(node as ts.NumericLiteral, members, parent);
    }
    default: {
      throw new Error('位置类型的UnaryExpression');
    }
  }
}

function handleTypeReferenceNode(
  node: ts.TypeReferenceNode,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  const typeName = handleEntityName(node.typeName, mockBuffer, members, parent, type);
  node.typeArguments?.forEach(
    typeArgument => handleTypeNode(mockBuffer, typeName.typeParameters, typeName, KeyValueTypes.REFERENCE, typeArgument)
  );
  members[typeName.key] = typeName;
  return typeName;
}

function handleExpression(
  node: ts.Expression,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  operateElements: KeyValue[]
): KeyValue {
  if (!node) {
    members[''] = generateKeyValue('', KeyValueTypes.VALUE, parent);
    return members[''];
  }
  let keyValue: KeyValue;
  switch (node.kind) {
    case SyntaxKind.Identifier: {
      keyValue = handleIdentifier(node as ts.Identifier, members, parent, type);
      operateElements.push(keyValue);
      break;
    }
    case SyntaxKind.NumericLiteral: {
      keyValue = handleNumericLiteral(node as ts.NumericLiteral, members, parent);
      operateElements.push(keyValue);
      break;
    }
    case SyntaxKind.PropertyAccessExpression: {
      keyValue = handlePropertyAccessExpression(node as ts.PropertyAccessExpression, mockBuffer, members, parent, type, operateElements);
      break;
    }
    case SyntaxKind.StringLiteral: {
      keyValue = handleStringLiteral(node as ts.StringLiteral, members, parent);
      operateElements.push(keyValue);
      break;
    }
    case SyntaxKind.ParenthesizedExpression: {
      keyValue = handleParenthesizedExpression(node as ts.ParenthesizedExpression, mockBuffer, members, parent, type, operateElements);
      break;
    }
    case SyntaxKind.BinaryExpression: {
      keyValue = handleBinaryExpression(node as ts.BinaryExpression, mockBuffer, members, parent);
      operateElements.push(keyValue);
      break;
    }
    case SyntaxKind.TrueKeyword: {
      keyValue = handleTrueKeyword(members, parent);
      operateElements.push(keyValue);
      break;
    }
    case SyntaxKind.FalseKeyword: {
      keyValue = handleFalseKeyword(members, parent);
      operateElements.push(keyValue);
      break;
    }
    default: {
      throw new Error('未知类型的Expression');
    }
  }
  return keyValue;
}

function handleIdentifier(
  node: ts.Identifier | ts.PrivateIdentifier,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes = KeyValueTypes.VALUE
): KeyValue {
  const text = node?.escapedText.toString() ?? '';
  const keyValue = generateKeyValue(text, type, parent);
  if (members[text] && members[text].sameName) {
    members[text].sameName.push(keyValue);
    return keyValue;
  }
  members[text] = keyValue;
  return keyValue;
}

function handleNumericLiteral(
  node: ts.NumericLiteral,
  members: Members,
  parent: KeyValue
): KeyValue {
  const text = node.text;
  members[text] = generateKeyValue(text, KeyValueTypes.VALUE, parent);
  return members[text];
}

function handlePropertyAccessExpression(
  node: ts.PropertyAccessExpression,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  operateElements: KeyValue[]
): KeyValue {
  const expression = handleExpression(node.expression, mockBuffer, members, parent, KeyValueTypes.REFERENCE, operateElements);

  switch (node.name.kind) {
    case SyntaxKind.Identifier: {
      expression.property = handleIdentifier(node.name as ts.Identifier, {}, expression, type);
      break;
    }
    case SyntaxKind.PrivateIdentifier: {
      expression.property = handlePrivateIdentifier(node.name as ts.PrivateIdentifier, {}, expression, type);
      break;
    }
    default: {
      throw new Error('未知类型的PropertyAccessExpression');
    }
  }
  return expression;
}

function handlePrivateIdentifier(
  node: ts.PrivateIdentifier,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes
): KeyValue {
  return handleIdentifier(node, members, parent, type);
}

function handleStringLiteral(
  node: ts.StringLiteral,
  members: Members,
  parent: KeyValue
): KeyValue {
  const text = node.text;
  members[text] = generateKeyValue(`'${text}'`, KeyValueTypes.VALUE, parent);
  return members[text];
}

function handleParenthesizedExpression(
  node: ts.ParenthesizedExpression,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  operateElements: KeyValue[]
): KeyValue {
  operateElements.push(generateKeyValue('(', KeyValueTypes.VALUE, parent));
  const expression = handleExpression(node.expression, mockBuffer, members, parent, type, operateElements);
  operateElements.push(generateKeyValue(')', KeyValueTypes.VALUE, parent));
  return expression;
}

function handleBinaryExpression(
  node: ts.BinaryExpression,
  mockBuffer: MockBuffer,
  members: Members,
  parent: KeyValue
): KeyValue {
  const expression = generateKeyValue('expression', KeyValueTypes.EXPRESSION, parent);
  members['expression'] = expression;
  expression.operateElements = [];

  handleExpression(node.left, mockBuffer, {}, parent, KeyValueTypes.VALUE, expression.operateElements);
  handleBinaryOperatorToken(node.operatorToken, {}, parent, KeyValueTypes.VALUE, expression.operateElements);
  handleExpression(node.right, mockBuffer, {}, parent, KeyValueTypes.VALUE, expression.operateElements);
  return expression;
}

function handleBinaryOperatorToken(
  node: ts.BinaryOperatorToken,
  members: Members,
  parent: KeyValue,
  type: KeyValueTypes,
  operateElements: KeyValue[]
): KeyValue {
  let operator: string;
  switch (node.kind) {
    case SyntaxKind.PlusToken: {
      operator = '+';
      break;
    }
    case SyntaxKind.LessThanLessThanToken: {
      operator = '<<';
      break;
    }
    case SyntaxKind.GreaterThanGreaterThanToken: {
      operator = '>>';
      break;
    }
    case SyntaxKind.BarToken: {
      operator = '|';
      break;
    }
    default: {
      throw new Error('位置类型的BinaryOperatorToken');
    }
  }
  const keyValue = generateKeyValue(operator, type, parent);
  operateElements.push(keyValue);
  return keyValue;
}

function handleTrueKeyword(
  members: Members,
  parent: KeyValue
): KeyValue {
  const keyValue = generateKeyValue('true', KeyValueTypes.VALUE, parent);
  members['\'true\''] = keyValue;
  return keyValue;
}

function handleFalseKeyword(
  members: Members,
  parent: KeyValue
): KeyValue {
  const keyValue = generateKeyValue('false', KeyValueTypes.VALUE, parent);
  members['\'false\''] = keyValue;
  return keyValue;
}
