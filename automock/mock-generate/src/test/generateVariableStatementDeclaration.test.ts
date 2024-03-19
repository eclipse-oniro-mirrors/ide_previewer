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

import { expect } from 'chai';
import { describe, it } from 'mocha';
import { generateVariableStatementDelcatation } from '../generate/generateVariableStatementDeclaration';

describe('generateVariableStatementDeclaration.ts file test', () => {
  it('Test statementEntity.typeKind is SyntaxKind.TypeReference', () => {
    const statementEntity = {
      statementName: 'daltonizationState',
      typeName: 'Config<boolean>',
      typeKind: 173,
      initializer: '',
    }
    const isInnerModule = false;
    const result = generateVariableStatementDelcatation(statementEntity, isInnerModule);
    expect(result).to.equal('daltonizationState: Config,');
  });

  it('Test statementEntity.typeKind is SyntaxKind.NumberKeyword', () => {
    const statementEntity = {
      statementName: 'batteryTemperature',
      typeName: 'number',
      typeKind: 144,
      initializer: '',
    }
    const isInnerModule = false;
    const result = generateVariableStatementDelcatation(statementEntity, isInnerModule);
    expect(result).to.equal('batteryTemperature: 0,');
  });

  it('Test statementEntity.typeKind is SyntaxKind.StringKeyword', () => {
    const statementEntity = {
      statementName: 'technology',
      typeName: 'string',
      typeKind: 147,
      initializer: '',
    }
    const isInnerModule = false;
    const result = generateVariableStatementDelcatation(statementEntity, isInnerModule);
    expect(result).to.equal('technology: \'\',');
  });

  it('Test statementEntity.typeKind is SyntaxKind.BooleanKeyword', () => {
    const statementEntity = {
      statementName: 'isBatteryPresent',
      typeName: 'boolean',
      typeKind: 131,
      initializer: '',
    }
    const isInnerModule = false;
    const result = generateVariableStatementDelcatation(statementEntity, isInnerModule);
    expect(result).to.equal('isBatteryPresent: true,');
  });

  it('Test statementEntity.typeKind is SyntaxKind.StringLiteral', () => {
    const statementEntity = {
      statementName: 'DATA_CHANGE_EVENT_ID',
      typeName: '',
      typeKind: 10,
      initializer: 'cloud_data_change',
    }
    const isInnerModule = false;
    const result = generateVariableStatementDelcatation(statementEntity, isInnerModule);
    expect(result).to.equal('DATA_CHANGE_EVENT_ID: cloud_data_change,');
  });

  it('Test statementEntity.typeKind is SyntaxKind.NumericLiteral', () => {
    const statementEntity = {
      statementName: 'MAX_KEY_LENGTH',
      typeName: '',
      typeKind: 8,
      initializer: '1024',
    }
    const isInnerModule = false;
    const result = generateVariableStatementDelcatation(statementEntity, isInnerModule);
    expect(result).to.equal('MAX_KEY_LENGTH: 1024,');
  });

  it('Test statementEntity.typeKind is SyntaxKind.LiteralType', () => {
    const statementEntity = {
      statementName: 'MAX_KEY_LENGTH',
      typeName: '80',
      typeKind: 191,
      initializer: '',
    }
    const isInnerModule = false;
    const result = generateVariableStatementDelcatation(statementEntity, isInnerModule);
    expect(result).to.equal('MAX_KEY_LENGTH: 80,');
  });
});