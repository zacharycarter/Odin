package all

// Imports every package
// This is useful for knowing what exists and producing documentation with `odin doc`

import bufio          "core:bufio"
import bytes          "core:bytes"

import c              "core:c"
import libc           "core:c/libc"

import compress       "core:compress"
import gzip           "core:compress/gzip"
import zlib           "core:compress/zlib"

import bit_array      "core:container/bit_array"
import priority_queue "core:container/priority_queue"
import queue          "core:container/queue"
import small_array    "core:container/small_array"
import lru            "core:container/lru"

import crypto           "core:crypto"
import blake            "core:crypto/blake"
import blake2b          "core:crypto/blake2b"
import blake2s          "core:crypto/blake2s"
import chacha20         "core:crypto/chacha20"
import chacha20poly1305 "core:crypto/chacha20poly1305"
import gost             "core:crypto/gost"
import groestl           "core:crypto/groestl"
import haval            "core:crypto/haval"
import jh               "core:crypto/jh"
import keccak           "core:crypto/keccak"
import md2              "core:crypto/md2"
import md4              "core:crypto/md4"
import md5              "core:crypto/md5"
import poly1305         "core:crypto/poly1305"
import ripemd           "core:crypto/ripemd"
import sha1             "core:crypto/sha1"
import sha2             "core:crypto/sha2"
import sha3             "core:crypto/sha3"
import shake            "core:crypto/shake"
import sm3              "core:crypto/sm3"
import streebog         "core:crypto/streebog"
import tiger            "core:crypto/tiger"
import tiger2           "core:crypto/tiger2"
import crypto_util      "core:crypto/util"
import whirlpool        "core:crypto/whirlpool"
import x25519           "core:crypto/x25519"

import dynlib         "core:dynlib"

import base32         "core:encoding/base32"
import base64         "core:encoding/base64"
import csv            "core:encoding/csv"
import hxa            "core:encoding/hxa"
import json           "core:encoding/json"

import fmt            "core:fmt"
import hash           "core:hash"

import image          "core:image"
import png            "core:image/png"

import io             "core:io"
import log            "core:log"

import math           "core:math"
import big            "core:math/big"
import bits           "core:math/bits"
import fixed          "core:math/fixed"
import linalg         "core:math/linalg"
import glm            "core:math/linalg/glsl"
import hlm            "core:math/linalg/hlsl"
import rand           "core:math/rand"

import mem            "core:mem"
// import virtual        "core:mem/virtual"

import ast            "core:odin/ast"
import doc_format     "core:odin/doc-format"
import odin_format    "core:odin/format"
import odin_parser    "core:odin/parser"
import odin_printer   "core:odin/printer"
import odin_tokenizer "core:odin/tokenizer"

import os             "core:os"

import slashpath      "core:path/slashpath"
import filepath       "core:path/filepath"

import reflect        "core:reflect"
import runtime        "core:runtime"
import slice          "core:slice"
import sort           "core:sort"
import strconv        "core:strconv"
import strings        "core:strings"
import sync           "core:sync"
import sync2          "core:sync/sync2"
import testing        "core:testing"
import scanner        "core:text/scanner"
import thread         "core:thread"
import time           "core:time"

import unicode        "core:unicode"
import utf8           "core:unicode/utf8"
import utf16          "core:unicode/utf16"

main :: proc(){}


_ :: bufio
_ :: bytes
_ :: c
_ :: libc
_ :: compress
_ :: gzip
_ :: zlib
_ :: bit_array
_ :: priority_queue
_ :: queue
_ :: small_array
_ :: lru
_ :: crypto
_ :: blake
_ :: blake2b
_ :: blake2s
_ :: chacha20
_ :: chacha20poly1305
_ :: gost
_ :: groestl
_ :: haval
_ :: jh
_ :: keccak
_ :: md2
_ :: md4
_ :: md5
_ :: poly1305
_ :: ripemd
_ :: sha1
_ :: sha2
_ :: sha3
_ :: shake
_ :: sm3
_ :: streebog
_ :: tiger
_ :: tiger2
_ :: crypto_util
_ :: whirlpool
_ :: x25519
_ :: dynlib
_ :: base32
_ :: base64
_ :: csv
_ :: hxa
_ :: json
_ :: fmt
_ :: hash
_ :: image
_ :: png
_ :: io
_ :: log
_ :: math
_ :: big
_ :: bits
_ :: fixed
_ :: linalg
_ :: glm
_ :: hlm
_ :: rand
_ :: mem
_ :: ast
_ :: doc_format
_ :: odin_format
_ :: odin_parser
_ :: odin_printer
_ :: odin_tokenizer
_ :: os
_ :: slashpath
_ :: filepath
_ :: reflect
_ :: runtime
_ :: slice
_ :: sort
_ :: strconv
_ :: strings
_ :: sync
_ :: sync2
_ :: testing
_ :: scanner
_ :: thread
_ :: time
_ :: unicode
_ :: utf8
_ :: utf16