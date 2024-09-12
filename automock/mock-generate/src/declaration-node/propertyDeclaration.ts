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

import type { GetAccessorDeclaration, PropertyDeclaration, SourceFile } from 'typescript';
import { getPropertyName } from '../common/commonUtils';

/**
 * get property node info
 * @param node
 * @param sourceFile
 * @returns
 */
export function getPropertyDeclaration(node: PropertyDeclaration, sourceFile: SourceFile): PropertyEntity {
  let propertyName = '';
  let propertyTypeName = '';
  let kind = -1;
  let isInitializer = false;
  let initializer = '';
  const modifiers: Array<string> = [];
  const fileText = sourceFile.getFullText();
  if (node.modifiers !== undefined) {
    node.modifiers.forEach(value => {
      modifiers.push(fileText.slice(value.pos, value.end));
    });
  }
  if (node.initializer !== undefined) {
    isInitializer = true;
    initializer = fileText.slice(node.initializer.pos, node.initializer.end).trim();
  }

  propertyName = getPropertyName(node.name, sourceFile);

  const propertyType = node.type;
  if (propertyType !== undefined) {
    propertyTypeName = fileText.slice(propertyType.pos, propertyType.end).trim();
    kind = propertyType.kind;
  }

  return {
    modifiers: modifiers,
    propertyName: propertyName,
    propertyTypeName: propertyTypeName,
    kind: kind,
    kinds: -1,
    isInitializer: isInitializer,
    initializer: initializer
  };
}

export function getGetDeclaration(node: GetAccessorDeclaration, sourceFile: SourceFile): PropertyEntity {
  let kind = -1;
  let kinds = -1;
  let propertyName = '';
  let propertyTypeName = '';
  const modifiers: Array<string> = [];
  const fileText = sourceFile.getFullText();

  if (node.modifiers !== undefined) {
    node.modifiers.forEach(value => {
      modifiers.push(fileText.slice(value.pos, value.end));
    });
  }

  propertyName = getPropertyName(node.name, sourceFile);
  const propertyType = node.type;
  if (propertyType !== undefined) {
    propertyTypeName = fileText.slice(propertyType.pos, propertyType.end).trim();
    kind = propertyType.kind;
    kinds = node.kind;
  }

  return {
    modifiers: modifiers,
    propertyName: propertyName,
    propertyTypeName: propertyTypeName,
    kind: kind,
    kinds: kinds,
    isInitializer: false,
    initializer: ''
  };
}

export interface PropertyEntity {
  modifiers: Array<string>;
  propertyName: string;
  propertyTypeName: string;
  kind: number;
  kinds: number;
  isInitializer: boolean;
  initializer: string;
}
