import fs from 'fs';
import path from 'path';
import { expect } from 'chai';
import { describe, it } from 'mocha';
import { createSourceFile, ScriptTarget } from 'typescript';
import { generateStaticFunction } from '../generate/generateStaticFunction';

describe('generateStaticFunction file test', () => {
  it('generateStaticFunction function test', () => {
    const filePath = path.join(__dirname, './api/global.d.ts')
    const code = fs.readFileSync(filePath);
    const sourceFile = createSourceFile(filePath, code.toString(), ScriptTarget.Latest);

    const staticMethod = {
      className: "console",
      methodEntity: {
        modifiers: [
          123,
        ],
        functionName: {
          name: "debug",
          expressionKind: -1,
          kind: 78,
        },
        returnType: {
          returnKindName: "void",
          returnKind: 113,
        },
        args: [
          {
            paramName: "message",
            paramTypeString: "string",
            paramTypeKind: 147,
          },
          {
            paramName: "arguments",
            paramTypeString: "any[]",
            paramTypeKind: 178,
          },
        ],
      },
    }
    const mockApi = `"import { TouchObject, KeyEvent, MouseEvent } from "../component/ets/common"`
    const data = `Console.debug = function(...args) {console.warn('The console.debug interface in the Previewer is a mocked implementation and may behave differently than on a real device.');
};`
    const result = generateStaticFunction(staticMethod, false, sourceFile, mockApi);
    console.log(result)
    
    expect(result).to.equal(data);
  });
});