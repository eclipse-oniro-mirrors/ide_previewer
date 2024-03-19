import fs from 'fs';
import path from 'path';
import { expect } from 'chai';
import { describe, it } from 'mocha';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generatePropertyDeclaration } from '../generate/generatePropertyDeclaration';

const filePath = path.join(__dirname, './api/@ohos.accessibility.d.ts')
const code = fs.readFileSync(filePath);
const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);

describe('generatePropertyDeclaration.ts file test', () => {
  it('Test propertyDeclaration.kind is SyntaxKind.TypeReference', () => {
    const rootName = 'EventInfo';
    const propertyDeclaration = {
      modifiers: [],
      propertyName: 'type',
      propertyTypeName: 'EventType',
      kind: 173,
      isInitializer: false,
      initializer: '',
    };
    const result = generatePropertyDeclaration(rootName, propertyDeclaration, sourceFile);
    expect(result).to.equal('this.type = EventType;');
  });

  it('Test propertyDeclaration.kind is SyntaxKind.StringKeyword', () => {
    const rootName = 'EventInfo';
    const propertyDeclaration = {
      modifiers: [],
      propertyName: 'bundleName',
      propertyTypeName: 'string',
      kind: 147,
      isInitializer: false,
      initializer: '',
    };
    const result = generatePropertyDeclaration(rootName, propertyDeclaration, sourceFile);
    expect(result).to.equal('this.bundleName = \'\'');
  });

  it('Test propertyDeclaration.kind is SyntaxKind.NumberKeyword', () => {
    const rootName = 'EventInfo';
    const propertyDeclaration = {
      modifiers: [],
      propertyName: 'pageId',
      propertyTypeName: 'number',
      kind: 144,
      isInitializer: false,
      initializer: '',
    };
    const result = generatePropertyDeclaration(rootName, propertyDeclaration, sourceFile);
    expect(result).to.equal('this.pageId = 0;');
  });
});