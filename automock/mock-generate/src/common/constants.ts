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

import path from 'path';
import { SyntaxKind } from 'typescript';
import { Declares, MockBuffer, mockedFilePath, rawFilePath } from '../types';
import { handleIterableIterator } from './commonUtils';

// api 路径参数所在索引
export const paramIndex = 2;

// 源文件到mock文件的映射
export const RawFileMap: Map<rawFilePath, mockedFilePath> = new Map<rawFilePath, mockedFilePath>();

// mock文件到源文件的映射
export const MockedFileMap: Map<mockedFilePath, rawFilePath> = new Map<mockedFilePath, rawFilePath>();

// 开源api接口文件列表
export const ohosDtsFileList: string[] = [];

// 导入全局声明类型的文件的列表
export const importDeclarationFiles: string[] = [];

// compoent 目录所有类型文件的列表
export const componentDtsFileList: string[] = [];

// arkts 目录所有类型文件的列表
export const arktsDtsFileList: string[] = [];

// kits 目录所有类型文件的列表
export const kitsDtsFileList: string[] = [];

// 源文件路径到mock信息的映射
export const mockBufferMap: Map<rawFilePath, MockBuffer> = new Map();

/**
 * 由于有些接口类型不需要直接mock，这会导致部分接口文件不用生成任何内容
 * 这导致引入这样的文件后，运行时会报错
 * 因此需要将空文件收集起来，以便后续进行忽略
 */
export const NO_CONTENT_FILES: Set<string> = new Set<string>();

// typescript 内置高级类型的mock方式
export const TSTypes = {
  Record: () => '{}',
  IterableIterator: (typeParams: string) => handleIterableIterator(typeParams),
  PropertyDecorator: () => '() => {}',
  ClassDecorator: () => '() => {}',
  MethodDecorator: () => '() => {}',
  Readonly: (params: string) => params,
  CanvasRenderingContext2D: () => '{name: \'The CanvasRenderingContext2D is not mocked\'}',
  WebGLRenderingContext: () => '{name: \'The WebGLRenderingContext is not mocked\'}',
  WebGL2RenderingContext: () => '{name: \'The WebGL2RenderingContext is not mocked\'}',
  thisType: () => '{}'
};

// 可以忽略的typescript内置类型
export const IGNORE_REFERENCES: Set<string> = new Set<string>([
  'Promise',
  'Partial',
  'Required',
  'Pick',
  'Exclude',
  'Extract',
  'Omit',
  'NonNullable'
]);

// 组件装饰器
export const COMPONENT_DECORATORS: string[] = ['CustomDialog', 'Component'];

// keyValue 节点类型
export enum KeyValueTypes {
  FUNCTION = 'FUNCTION',
  CLASS = 'CLASS',
  VALUE = 'VALUE',
  FILE = 'FILE',
  MODULE = 'MODULE',
  INTERFACE = 'INTERFACE',
  IMPORT = 'IMPORT',
  EXPORT = 'EXPORT',
  INTERSECTION = 'INTERSECTION',
  VARIABLE = 'VARIABLE',
  REFERENCE = 'REFERENCE',
  PROPERTY = 'PROPERTY',
  ENUM = 'ENUM',
  EXPRESSION = 'EXPRESSION'
}

// napi 目录
export const NAPI_DIR_PATH = path.join(__dirname, '..', '..', '..', 'runtime', 'main', 'extend', 'systemplugin', 'napi');

// 全局声明的类型集
export const DECLARES: Declares = {};

// typescript的基础类型
export const BASE_KINDS: SyntaxKind[] = [
  SyntaxKind.AnyKeyword,
  SyntaxKind.BigIntKeyword,
  SyntaxKind.BooleanKeyword,
  SyntaxKind.IntrinsicKeyword,
  SyntaxKind.NeverKeyword,
  SyntaxKind.NumberKeyword,
  SyntaxKind.ObjectKeyword,
  SyntaxKind.StringKeyword,
  SyntaxKind.SymbolKeyword,
  SyntaxKind.UndefinedKeyword,
  SyntaxKind.UnknownKeyword,
  SyntaxKind.VoidKeyword,
  SyntaxKind.NullKeyword,
  SyntaxKind.LiteralType
];

// typescript 关键字
export const TYPESCRIPT_KEYWORDS: Set<string> = new Set([
  'abstract', 'any', 'as', 'async', 'await', 'boolean', 'break', 'byte', 'case', 'catch', 'class', 'const',
  'constructor', 'continue', 'debugger', 'declare', 'default', 'delete', 'do', 'double', 'else', 'enum',
  'export', 'extends', 'false', 'finally', 'float', 'for', 'from', 'function', 'get', 'global', 'goto',
  'if', 'implements', 'import', 'in', 'instanceof', 'int', 'interface', 'is', 'let', 'long', 'map', 'module',
  'namespace', 'native', 'new', 'null', 'of', 'package', 'private', 'protected', 'public', 'readonly',
  'require', 'return', 'set', 'short', 'static', 'super', 'switch', 'synchronized', 'this', 'throw', 'throws',
  'transient', 'true', 'try', 'typeof', 'var', 'void', 'volatile', 'while', 'with', 'yield', 'async', 'await',
  'declare', 'readonly', 'assert', 'unknown', 'partial', 'readonlyRecord', 'override', 'infer', 'satisfies',
  'as const', 'is unknown', 'is any', 'is never', 'checks', 'constraint', 'covariant', 'contravariant', 'invariant'
]);

// .d.ts 文件后缀
export const D_TS: string = '.d.ts';

// .d.ets 文件后缀
export const D_ETS: string = '.d.ets';
