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

import { KeyValueTypes } from '../common/constants';

export interface KeyValue {
  key: string,
  type: KeyValueTypes,
  members: Members,
  typeParameters: Members,
  methodParams: Members,
  constraint: Members,
  sameName: KeyValue[],
  importedModulePath?: string,
  heritage?: KeyValue,
  property?: KeyValue,
  parent?: KeyValue,
  isExport?: boolean,
  isDefault?: boolean,
  isImportDefault?: boolean,
  isNamespaceImport?: boolean,
  isStatic?: boolean,
  value?: string,
  rawName?: string,
  operateElements?: KeyValue[],
  isNeedMock?: boolean,
  isArrowFunction?: boolean,
  isGlobalDeclare?: boolean,
  dependOnGlobals: Set<KeyValue>,
  isMocked?: boolean
}

export interface Members {
  [key: string]: KeyValue
}

export interface MockBuffer {
  contents: KeyValue,
  mockedFilePath: string,
  rawFilePath: string,
}

export interface Declares {
  [key: string]: Declare,
}

export interface Declare {
  keyValue: KeyValue,
  from: string
}

export type ApiFolder = 'api' | 'component' | 'arkts' | 'kits';
