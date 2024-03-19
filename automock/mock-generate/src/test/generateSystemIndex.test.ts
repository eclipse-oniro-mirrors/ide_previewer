import { expect } from 'chai';
import { describe, it } from 'mocha';
import { generateSystemIndex } from '../generate/generateSystemIndex';

describe('generateSystemIndex file test', () => {
  it('generateSystemIndex function test', () => {
    const data = `import regeneratorRuntime from 'babel-runtime/regenerator'
import {mockRequireNapiFun} from './napi/index';
;(function mockSystemPlugin() {
    global.regeneratorRuntime = regeneratorRuntime
    global.systemplugin = {}
    global.ohosplugin = {}
mockRequireNapiFun();
}());`
    const result = generateSystemIndex();
    expect(result).to.equal(data);
  });
});