import { expect } from 'chai';
import { describe, it } from 'mocha';
import { generateIndexSignature } from '../generate/generateIndexSignature';

describe('generateIndexSignature.ts file test', () => {
  it('Test the generateIndexSignature function', () => {
    const signatureEntity = {
      indexSignatureKey: 'key',
      indexSignatureKind: 182,
      indexSignatureTypeName: 'number | boolean | string | undefined',
    };
    const result = generateIndexSignature(signatureEntity);
    expect(result).to.equal('key: \'[PC Preview] unknown type\',\n');
  });

  it('Test the generateIndexSignature function', () => {
    const signatureEntity = {
      indexSignatureKey: 'key',
      indexSignatureKind: 173,
      indexSignatureTypeName: 'BundleStateInfo',
    };
    const result = generateIndexSignature(signatureEntity);
    expect(result).to.equal('key: BundleStateInfo');
  });
});