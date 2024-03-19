import fs from 'fs';
import path from 'path';
import { expect } from 'chai';
import { describe, it } from 'mocha';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generateInterfaceDeclaration } from '../generate/generateInterfaceDeclaration';

describe('generateInterfaceDeclaration.ts file test', () => {
  it('Test the generateInterfaceDeclaration function', () => {
    const filePath = path.join(__dirname, './api/@ohos.abilityAccessCtrl.d.ts')
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);
    const interfaceEntity = {
      interfaceName: 'AtManager',
      typeParameters: [],
      heritageClauses: [],
      interfaceConstructors: [],
      interfaceMethodSignature: new Map(),
      interfacePropertySignatures: [],
      callSignatures: [],
      indexSignature: [],
      exportModifiers: [],
    };
    const isSourceFile = false;
    const mockApi = 'import { AsyncCallback, Callback } from \'./ohos_base\''
      + 'import { Permissions } from \'./permissions\''
      + 'import _Context from \'./application/Context\''
      + 'import _PermissionRequestResult from \'./security/PermissionRequestResult\''
      + 'export const PermissionRequestResult = new _PermissionRequestResult();'
      + 'export const Context = _Context;';
    const currentSourceInterfaceArray = [
      {
        interfaceName: 'AtManager',
        typeParameters: [],
        heritageClauses: [],
        interfaceConstructors: [],
        interfaceMethodSignature: new Map(),
        interfacePropertySignatures: [],
        callSignatures: [],
        indexSignature: [],
        exportModifiers: [],
      },
      {
        interfaceName: 'PermissionStateChangeInfo',
        typeParameters: [],
        heritageClauses: [],
        interfaceConstructors: [],
        interfaceMethodSignature: new Map(),
        interfacePropertySignatures: [],
        callSignatures: [],
        indexSignature: [],
        exportModifiers: [],
      },
    ];
    const importDeclarations = [
      {
        importPath: './@ohos.base',
        importElements: '{ AsyncCallback, Callback }',
      },
      {
        importPath: './permissions',
        importElements: '{ Permissions }',
      },
      {
        importPath: './application/Context',
        importElements: '_Context',
      },
      {
        importPath: './security/PermissionRequestResult',
        importElements: '_PermissionRequestResult',
      },
    ];
    const extraImport = [];
    const result = generateInterfaceDeclaration(
      interfaceEntity,
      sourceFile,
      isSourceFile,
      mockApi,
      currentSourceInterfaceArray,
      importDeclarations,
      extraImport
    );
    expect(result).to.equal('const AtManager = { \n}\n');
  });
});