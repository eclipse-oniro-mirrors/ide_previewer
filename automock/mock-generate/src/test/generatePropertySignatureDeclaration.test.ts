import fs from 'fs';
import path from 'path';
import { expect } from 'chai';
import { describe, it } from 'mocha';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generatePropertySignatureDeclaration } from '../generate/generatePropertySignatureDeclaration';

const filePath = path.join(__dirname, './api/@ohos.abilityAccessCtrl.d.ts')
const code = fs.readFileSync(filePath);
const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);

describe('generatePropertySignatureDeclaration.ts file test', () => {
  it('Test propertySignature.kind is SyntaxKind.TypeReference', () => {
    const rootName = 'PermissionStateChangeInfo';
    const propertySignature = {
      modifiers: [],
      propertyName: 'change',
      propertyTypeName: 'PermissionStateChangeType',
      kind: 173,
    };
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { Permissions } from \'./permissions\''
      + 'import _Context from \'./application/Context\''
      + 'import _PermissionRequestResult from \'./security/PermissionRequestResult\''
      + 'export const PermissionRequestResult = new _PermissionRequestResult();'
      + 'export const Context = _Context;';
    const result = generatePropertySignatureDeclaration(rootName, propertySignature, sourceFile, mockApi);
    expect(result).to.equal('change: PermissionStateChangeType,');
  });
  it('Test propertySignature.kind is SyntaxKind.NumberKeyword', () => {
    const rootName = 'PermissionStateChangeInfo';
    const propertySignature = {
      modifiers: [],
      propertyName: 'tokenID',
      propertyTypeName: 'number',
      kind: 144,
    };
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { Permissions } from \'./permissions\''
      + 'import _Context from \'./application/Context\''
      + 'import _PermissionRequestResult from \'./security/PermissionRequestResult\''
      + 'export const PermissionRequestResult = new _PermissionRequestResult();'
      + 'export const Context = _Context;';
    const result = generatePropertySignatureDeclaration(rootName, propertySignature, sourceFile, mockApi);
    expect(result).to.equal('tokenID: 0,');
  });
});