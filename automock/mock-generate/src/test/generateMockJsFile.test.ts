import fs from 'fs';
import path from 'path';
import { expect } from 'chai';
import { describe, it } from 'mocha';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generateImportDeclaration, referenctImport2ModuleImport } from '../generate/generateMockJsFile';

describe('generateModuleDeclaration.ts file test', () => {
  it('Test the generateImportDeclaration function', () => {
    const filePath = path.join(__dirname, './api/@ohos.ability.ability.d.ts');
    const importEntity = {
      importPath: 'SpecialEvent',
      importElements: '{ TouchObject, KeyEvent, MouseEvent }',
    };
    const sourceFileName = 'ohos_ability_ability';
    const heritageClausesArray = [];
    const sourceFileList = [];
    const result = generateImportDeclaration(importEntity, sourceFileName, heritageClausesArray, filePath, sourceFileList);
    expect(result).to.equal('');
  });

  it('Test the referenctImport2ModuleImport function', () => {
    const importEntity = {
      importPath: './ability/dataAbilityHelper',
      importElements: '\'{ DataAbilityHelper as _DataAbilityHelper }\'',
    };
    const currentFilePath = path.join(__dirname, './api/global.d.ts');
    const code = fs.readFileSync(currentFilePath);
    const sourceFileList = [];
    sourceFileList.push(createSourceFile(currentFilePath, code.toString(), ScriptTarget.Latest));
    const result = referenctImport2ModuleImport(importEntity, currentFilePath, sourceFileList);
    expect(result).to.equal('');
  });
});