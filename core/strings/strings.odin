package strings

import "core:io"
import "core:mem"
import "core:unicode"
import "core:unicode/utf8"

clone :: proc(s: string, allocator := context.allocator, loc := #caller_location) -> string {
	c := make([]byte, len(s), allocator, loc)
	copy(c, s)
	return string(c[:len(s)])
}

clone_to_cstring :: proc(s: string, allocator := context.allocator, loc := #caller_location) -> cstring {
	c := make([]byte, len(s)+1, allocator, loc)
	copy(c, s)
	c[len(s)] = 0
	return cstring(&c[0])
}

string_from_ptr :: proc(ptr: ^byte, len: int) -> string {
	return transmute(string)mem.Raw_String{ptr, len}
}

string_from_nul_terminated_ptr :: proc(ptr: ^byte, len: int) -> string {
	s := transmute(string)mem.Raw_String{ptr, len}
	s = truncate_to_byte(s, 0)
	return s
}


ptr_from_string :: proc(str: string) -> ^byte {
	d := transmute(mem.Raw_String)str
	return d.data
}

unsafe_string_to_cstring :: proc(str: string) -> cstring {
	d := transmute(mem.Raw_String)str
	return cstring(d.data)
}

truncate_to_byte :: proc(str: string, b: byte) -> string {
	n := index_byte(str, b)
	if n < 0 {
		n = len(str)
	}
	return str[:n]
}
truncate_to_rune :: proc(str: string, r: rune) -> string {
	n := index_rune(str, r)
	if n < 0 {
		n = len(str)
	}
	return str[:n]
}

clone_from_bytes :: proc(s: []byte, allocator := context.allocator, loc := #caller_location) -> string {
	c := make([]byte, len(s)+1, allocator, loc)
	copy(c, s)
	c[len(s)] = 0
	return string(c[:len(s)])
}
clone_from_cstring :: proc(s: cstring, allocator := context.allocator, loc := #caller_location) -> string {
	return clone(string(s), allocator, loc)
}
clone_from_ptr :: proc(ptr: ^byte, len: int, allocator := context.allocator, loc := #caller_location) -> string {
	s := string_from_ptr(ptr, len)
	return clone(s, allocator, loc)
}

clone_from :: proc{
	clone,
	clone_from_bytes,
	clone_from_cstring,
	clone_from_ptr,
}

clone_from_cstring_bounded :: proc(ptr: cstring, len: int, allocator := context.allocator, loc := #caller_location) -> string {
	s := string_from_ptr((^u8)(ptr), len)
	s = truncate_to_byte(s, 0)
	return clone(s, allocator, loc)
}

// Compares two strings, returning a value representing which one comes first lexiographically.
// -1 for `a`; 1 for `b`, or 0 if they are equal.
compare :: proc(lhs, rhs: string) -> int {
	return mem.compare(transmute([]byte)lhs, transmute([]byte)rhs)
}

contains_rune :: proc(s: string, r: rune) -> int {
	for c, offset in s {
		if c == r {
			return offset
		}
	}
	return -1
}

contains :: proc(s, substr: string) -> bool {
	return index(s, substr) >= 0
}

contains_any :: proc(s, chars: string) -> bool {
	return index_any(s, chars) >= 0
}


rune_count :: proc(s: string) -> int {
	return utf8.rune_count_in_string(s)
}


equal_fold :: proc(u, v: string) -> bool {
	s, t := u, v
	loop: for s != "" && t != "" {
		sr, tr: rune
		if s[0] < utf8.RUNE_SELF {
			sr, s = rune(s[0]), s[1:]
		} else {
			r, size := utf8.decode_rune_in_string(s)
			sr, s = r, s[size:]
		}
		if t[0] < utf8.RUNE_SELF {
			tr, t = rune(t[0]), t[1:]
		} else {
			r, size := utf8.decode_rune_in_string(t)
			tr, t = r, t[size:]
		}

		if tr == sr { // easy case
			continue loop
		}

		if tr < sr {
			tr, sr = sr, tr
		}

		if tr < utf8.RUNE_SELF {
			switch sr {
			case 'A'..='Z':
				if tr == (sr+'a')-'A' {
					continue loop
				}
			}
			return false
		}

		// TODO(bill): Unicode folding

		return false
	}

	return s == t
}

has_prefix :: proc(s, prefix: string) -> bool {
	return len(s) >= len(prefix) && s[0:len(prefix)] == prefix
}

has_suffix :: proc(s, suffix: string) -> bool {
	return len(s) >= len(suffix) && s[len(s)-len(suffix):] == suffix
}


join :: proc(a: []string, sep: string, allocator := context.allocator) -> string {
	if len(a) == 0 {
		return ""
	}

	n := len(sep) * (len(a) - 1)
	for s in a {
		n += len(s)
	}

	b := make([]byte, n, allocator)
	i := copy(b, a[0])
	for s in a[1:] {
		i += copy(b[i:], sep)
		i += copy(b[i:], s)
	}
	return string(b)
}

concatenate :: proc(a: []string, allocator := context.allocator) -> string {
	if len(a) == 0 {
		return ""
	}

	n := 0
	for s in a {
		n += len(s)
	}
	b := make([]byte, n, allocator)
	i := 0
	for s in a {
		i += copy(b[i:], s)
	}
	return string(b)
}

/*
	`rune_offset` and `rune_length` are in runes, not bytes.
	If `rune_length` <= 0, then it'll return the remainder of the string starting with `rune_offset`.
*/
cut :: proc(s: string, rune_offset := int(0), rune_length := int(0), allocator := context.allocator) -> (res: string) {
	s := s; rune_length := rune_length
	l := utf8.rune_count_in_string(s)

	if rune_offset >= l { return "" }
	if rune_offset == 0 && rune_length <= 0 {
		return clone(s, allocator)
	}
	if rune_length == 0 { rune_length = l }

	bytes_needed := min(rune_length * 4, len(s))
	buf := make([]u8, bytes_needed, allocator)

	byte_offset := 0
	for i := 0; i < l; i += 1 {
		_, w := utf8.decode_rune_in_string(s)
		if i >= rune_offset {
			for j := 0; j < w; j += 1 {
				buf[byte_offset+j] = s[j]
			}
			byte_offset += w
		}
		if rune_length > 0 {
			if i == rune_offset + rune_length - 1 { break }
		}
		s = s[w:]
	}
	return string(buf[:byte_offset])
}

@private
_split :: proc(s_, sep: string, sep_save, n_: int, allocator := context.allocator) -> []string {
	s, n := s_, n_

	if n == 0 {
		return nil
	}

	if sep == "" {
		l := utf8.rune_count_in_string(s)
		if n < 0 || n > l {
			n = l
		}

		res := make([dynamic]string, n, allocator)
		for i := 0; i < n-1; i += 1 {
			_, w := utf8.decode_rune_in_string(s)
			res[i] = s[:w]
			s = s[w:]
		}
		if n > 0 {
			res[n-1] = s
		}
		return res[:]
	}

	if n < 0 {
		n = count(s, sep) + 1
	}

	res := make([dynamic]string, n, allocator)

	n -= 1

	i := 0
	for ; i < n; i += 1 {
		m := index(s, sep)
		if m < 0 {
			break
		}
		res[i] = s[:m+sep_save]
		s = s[m+len(sep):]
	}
	res[i] = s

	return res[:i+1]
}

split :: proc(s, sep: string, allocator := context.allocator) -> []string {
	return _split(s, sep, 0, -1, allocator)
}

split_n :: proc(s, sep: string, n: int, allocator := context.allocator) -> []string {
	return _split(s, sep, 0, n, allocator)
}

split_after :: proc(s, sep: string, allocator := context.allocator) -> []string {
	return _split(s, sep, len(sep), -1, allocator)
}

split_after_n :: proc(s, sep: string, n: int, allocator := context.allocator) -> []string {
	return _split(s, sep, len(sep), n, allocator)
}


@private
_split_iterator :: proc(s: ^string, sep: string, sep_save: int) -> (res: string, ok: bool) {
	if sep == "" {
		res = s[:]
		ok = true
		s^ = s[len(s):]
		return
	}

	m := index(s^, sep)
	if m < 0 {
		// not found
		res = s[:]
		ok = res != ""
		s^ = s[len(s):]
	} else {
		res = s[:m+sep_save]
		ok = true
		s^ = s[m+len(sep):]
	}
	return
}


split_iterator :: proc(s: ^string, sep: string) -> (string, bool) {
	return _split_iterator(s, sep, 0)
}

split_after_iterator :: proc(s: ^string, sep: string) -> (string, bool) {
	return _split_iterator(s, sep, len(sep))
}


@(private)
_trim_cr :: proc(s: string) -> string {
	n := len(s)
	if n > 0 {
		if s[n-1] == '\r' {
			return s[:n-1]
		}
	}
	return s
}

split_lines :: proc(s: string, allocator := context.allocator) -> []string {
	sep :: "\n"
	lines := _split(s, sep, 0, -1, allocator)
	for line in &lines {
		line = _trim_cr(line)
	}
	return lines
}

split_lines_n :: proc(s: string, n: int, allocator := context.allocator) -> []string {
	sep :: "\n"
	lines := _split(s, sep, 0, n, allocator)
	for line in &lines {
		line = _trim_cr(line)
	}
	return lines
}

split_lines_after :: proc(s: string, allocator := context.allocator) -> []string {
	sep :: "\n"
	lines := _split(s, sep, len(sep), -1, allocator)
	for line in &lines {
		line = _trim_cr(line)
	}
	return lines
}

split_lines_after_n :: proc(s: string, n: int, allocator := context.allocator) -> []string {
	sep :: "\n"
	lines := _split(s, sep, len(sep), n, allocator)
	for line in &lines {
		line = _trim_cr(line)
	}
	return lines
}

split_lines_iterator :: proc(s: ^string) -> (line: string, ok: bool) {
	sep :: "\n"
	line = _split_iterator(s, sep, 0) or_return
	return _trim_cr(line), true
}

split_lines_after_iterator :: proc(s: ^string) -> (line: string, ok: bool) {
	sep :: "\n"
	line = _split_iterator(s, sep, len(sep)) or_return
	return _trim_cr(line), true
}




index_byte :: proc(s: string, c: byte) -> int {
	for i := 0; i < len(s); i += 1 {
		if s[i] == c {
			return i
		}
	}
	return -1
}

// Returns -1 if c is not present
last_index_byte :: proc(s: string, c: byte) -> int {
	for i := len(s)-1; i >= 0; i -= 1 {
		if s[i] == c {
			return i
		}
	}
	return -1
}



@private PRIME_RABIN_KARP :: 16777619

index :: proc(s, substr: string) -> int {
	hash_str_rabin_karp :: proc(s: string) -> (hash: u32 = 0, pow: u32 = 1) {
		for i := 0; i < len(s); i += 1 {
			hash = hash*PRIME_RABIN_KARP + u32(s[i])
		}
		sq := u32(PRIME_RABIN_KARP)
		for i := len(s); i > 0; i >>= 1 {
			if (i & 1) != 0 {
				pow *= sq
			}
			sq *= sq
		}
		return
	}

	n := len(substr)
	switch {
	case n == 0:
		return 0
	case n == 1:
		return index_byte(s, substr[0])
	case n == len(s):
		if s == substr {
			return 0
		}
		return -1
	case n > len(s):
		return -1
	}

	hash, pow := hash_str_rabin_karp(substr)
	h: u32
	for i := 0; i < n; i += 1 {
		h = h*PRIME_RABIN_KARP + u32(s[i])
	}
	if h == hash && s[:n] == substr {
		return 0
	}
	for i := n; i < len(s); /**/ {
		h *= PRIME_RABIN_KARP
		h += u32(s[i])
		h -= pow * u32(s[i-n])
		i += 1
		if h == hash && s[i-n:i] == substr {
			return i - n
		}
	}
	return -1
}

last_index :: proc(s, substr: string) -> int {
	hash_str_rabin_karp_reverse :: proc(s: string) -> (hash: u32 = 0, pow: u32 = 1) {
		for i := len(s) - 1; i >= 0; i -= 1 {
			hash = hash*PRIME_RABIN_KARP + u32(s[i])
		}
		sq := u32(PRIME_RABIN_KARP)
		for i := len(s); i > 0; i >>= 1 {
			if (i & 1) != 0 {
				pow *= sq
			}
			sq *= sq
		}
		return
	}

	n := len(substr)
	switch {
	case n == 0:
		return len(s)
	case n == 1:
		return last_index_byte(s, substr[0])
	case n == len(s):
		return 0 if substr == s else -1
	case n > len(s):
		return -1
	}

	hash, pow := hash_str_rabin_karp_reverse(substr)
	last := len(s) - n
	h: u32
	for i := len(s)-1; i >= last; i -= 1 {
		h = h*PRIME_RABIN_KARP + u32(s[i])
	}
	if h == hash && s[last:] == substr {
		return last
	}

	for i := last-1; i >= 0; i -= 1 {
		h *= PRIME_RABIN_KARP
		h += u32(s[i])
		h -= pow * u32(s[i+n])
		if h == hash && s[i:i+n] == substr {
			return i
		}
	}
	return -1
}

// index_any returns the index of the first char of `chars` found in `s`. -1 if not found.
index_any :: proc(s, chars: string) -> int {
	if chars == "" {
		return -1
	}
	
	if len(chars) == 1 {
		r := rune(chars[0])
		if r >= utf8.RUNE_SELF {
			r = utf8.RUNE_ERROR
		}
		return index_rune(s, r)
	}
	
	if len(s) > 8 {
		if as, ok := ascii_set_make(chars); ok {
			for i in 0..<len(s) {
				if ascii_set_contains(as, s[i]) {
					return i
				}
			}
			return -1
		}
	}

	for c, i in s {
		if index_rune(chars, c) >= 0 {
			return i
		}
	}
	return -1
}

last_index_any :: proc(s, chars: string) -> int {
	if chars == "" {
		return -1
	}
	
	if len(s) == 1 {
		r := rune(s[0])
		if r >= utf8.RUNE_SELF {
			r = utf8.RUNE_ERROR
		}
		return index_rune(chars, r)
	}
	
	if len(s) > 8 {
		if as, ok := ascii_set_make(chars); ok {
			for i := len(s)-1; i >= 0; i -= 1 {
				if ascii_set_contains(as, s[i]) {
					return i
				}
			}
			return -1
		}
	}
	
	if len(chars) == 1 {
		r := rune(chars[0])
		if r >= utf8.RUNE_SELF {
			r = utf8.RUNE_ERROR
		}
		for i := len(s); i > 0; /**/ {
			c, w := utf8.decode_last_rune_in_string(s[:i])
			i -= w
			if c == r {
				return i
			}
		}
		return -1
	}

	for i := len(s); i > 0; /**/ {
		r, w := utf8.decode_last_rune_in_string(s[:i])
		i -= w
		if index_rune(chars, r) >= 0 {
			return i
		}
	}
	return -1
}

count :: proc(s, substr: string) -> int {
	if len(substr) == 0 { // special case
		return rune_count(s) + 1
	}
	if len(substr) == 1 {
		c := substr[0]
		switch len(s) {
		case 0:
			return 0
		case 1:
			return int(s[0] == c)
		}
		n := 0
		for i := 0; i < len(s); i += 1 {
			if s[i] == c {
				n += 1
			}
		}
		return n
	}

	// TODO(bill): Use a non-brute for approach
	n := 0
	str := s
	for {
		i := index(str, substr)
		if i == -1 {
			return n
		}
		n += 1
		str = str[i+len(substr):]
	}
	return n
}


repeat :: proc(s: string, count: int, allocator := context.allocator) -> string {
	if count < 0 {
		panic("strings: negative repeat count")
	} else if count > 0 && (len(s)*count)/count != len(s) {
		panic("strings: repeat count will cause an overflow")
	}

	b := make([]byte, len(s)*count, allocator)
	i := copy(b, s)
	for i < len(b) { // 2^N trick to reduce the need to copy
		copy(b[i:], b[:i])
		i *= 2
	}
	return string(b)
}

replace_all :: proc(s, old, new: string, allocator := context.allocator) -> (output: string, was_allocation: bool) {
	return replace(s, old, new, -1, allocator)
}

// if n < 0, no limit on the number of replacements
replace :: proc(s, old, new: string, n: int, allocator := context.allocator) -> (output: string, was_allocation: bool) {
	if old == new || n == 0 {
		was_allocation = false
		output = s
		return
	}
	byte_count := n
	if m := count(s, old); m == 0 {
		was_allocation = false
		output = s
		return
	} else if n < 0 || m < n {
		byte_count = m
	}


	t := make([]byte, len(s) + byte_count*(len(new) - len(old)), allocator)
	was_allocation = true

	w := 0
	start := 0
	for i := 0; i < byte_count; i += 1 {
		j := start
		if len(old) == 0 {
			if i > 0 {
				_, width := utf8.decode_rune_in_string(s[start:])
				j += width
			}
		} else {
			j += index(s[start:], old)
		}
		w += copy(t[w:], s[start:j])
		w += copy(t[w:], new)
		start = j + len(old)
	}
	w += copy(t[w:], s[start:])
	output = string(t[0:w])
	return
}

remove :: proc(s, key: string, n: int, allocator := context.allocator) -> (output: string, was_allocation: bool) {
	return replace(s, key, "", n, allocator)
}

remove_all :: proc(s, key: string, allocator := context.allocator) -> (output: string, was_allocation: bool) {
	return remove(s, key, -1, allocator)
}

@(private) _ascii_space := [256]bool{'\t' = true, '\n' = true, '\v' = true, '\f' = true, '\r' = true, ' ' = true}


is_ascii_space :: proc(r: rune) -> bool {
	if r < utf8.RUNE_SELF {
		return _ascii_space[u8(r)]
	}
	return false
}

is_space :: proc(r: rune) -> bool {
	if r < 0x2000 {
		switch r {
		case '\t', '\n', '\v', '\f', '\r', ' ', 0x85, 0xa0, 0x1680:
			return true
		}
	} else {
		if r <= 0x200a {
			return true
		}
		switch r {
		case 0x2028, 0x2029, 0x202f, 0x205f, 0x3000:
			return true
		}
	}
	return false
}

is_null :: proc(r: rune) -> bool {
	return r == 0x0000
}

index_proc :: proc(s: string, p: proc(rune) -> bool, truth := true) -> int {
	for r, i in s {
		if p(r) == truth {
			return i
		}
	}
	return -1
}

index_proc_with_state :: proc(s: string, p: proc(rawptr, rune) -> bool, state: rawptr, truth := true) -> int {
	for r, i in s {
		if p(state, r) == truth {
			return i
		}
	}
	return -1
}

last_index_proc :: proc(s: string, p: proc(rune) -> bool, truth := true) -> int {
	// TODO(bill): Probably use Rabin-Karp Search
	for i := len(s); i > 0; {
		r, size := utf8.decode_last_rune_in_string(s[:i])
		i -= size
		if p(r) == truth {
			return i
		}
	}
	return -1
}

last_index_proc_with_state :: proc(s: string, p: proc(rawptr, rune) -> bool, state: rawptr, truth := true) -> int {
	// TODO(bill): Probably use Rabin-Karp Search
	for i := len(s); i > 0; {
		r, size := utf8.decode_last_rune_in_string(s[:i])
		i -= size
		if p(state, r) == truth {
			return i
		}
	}
	return -1
}

trim_left_proc :: proc(s: string, p: proc(rune) -> bool) -> string {
	i := index_proc(s, p, false)
	if i == -1 {
		return ""
	}
	return s[i:]
}


index_rune :: proc(s: string, r: rune) -> int {
	switch {
	case 0 <= r && r < utf8.RUNE_SELF:
		return index_byte(s, byte(r))

	case r == utf8.RUNE_ERROR:
		for c, i in s {
			if c == utf8.RUNE_ERROR {
				return i
			}
		}
		return -1

	case !utf8.valid_rune(r):
		return -1
	}

	b, w := utf8.encode_rune(r)
	return index(s, string(b[:w]))
}


trim_left_proc_with_state :: proc(s: string, p: proc(rawptr, rune) -> bool, state: rawptr) -> string {
	i := index_proc_with_state(s, p, state, false)
	if i == -1 {
		return ""
	}
	return s[i:]
}

trim_right_proc :: proc(s: string, p: proc(rune) -> bool) -> string {
	i := last_index_proc(s, p, false)
	if i >= 0 && s[i] >= utf8.RUNE_SELF {
		_, w := utf8.decode_rune_in_string(s[i:])
		i += w
	} else {
		i += 1
	}
	return s[0:i]
}

trim_right_proc_with_state :: proc(s: string, p: proc(rawptr, rune) -> bool, state: rawptr) -> string {
	i := last_index_proc_with_state(s, p, state, false)
	if i >= 0 && s[i] >= utf8.RUNE_SELF {
		_, w := utf8.decode_rune_in_string(s[i:])
		i += w
	} else {
		i += 1
	}
	return s[0:i]
}


is_in_cutset :: proc(state: rawptr, r: rune) -> bool {
	if state == nil {
		return false
	}
	cutset := (^string)(state)^
	for c in cutset {
		if r == c {
			return true
		}
	}
	return false
}


trim_left :: proc(s: string, cutset: string) -> string {
	if s == "" || cutset == "" {
		return s
	}
	state := cutset
	return trim_left_proc_with_state(s, is_in_cutset, &state)
}

trim_right :: proc(s: string, cutset: string) -> string {
	if s == "" || cutset == "" {
		return s
	}
	state := cutset
	return trim_right_proc_with_state(s, is_in_cutset, &state)
}

trim :: proc(s: string, cutset: string) -> string {
	return trim_right(trim_left(s, cutset), cutset)
}

trim_left_space :: proc(s: string) -> string {
	return trim_left_proc(s, is_space)
}

trim_right_space :: proc(s: string) -> string {
	return trim_right_proc(s, is_space)
}

trim_space :: proc(s: string) -> string {
	return trim_right_space(trim_left_space(s))
}


trim_left_null :: proc(s: string) -> string {
	return trim_left_proc(s, is_null)
}

trim_right_null :: proc(s: string) -> string {
	return trim_right_proc(s, is_null)
}

trim_null :: proc(s: string) -> string {
	return trim_right_null(trim_left_null(s))
}

trim_prefix :: proc(s, prefix: string) -> string {
	if has_prefix(s, prefix) {
		return s[len(prefix):]
	}
	return s
}

trim_suffix :: proc(s, suffix: string) -> string {
	if has_suffix(s, suffix) {
		return s[:len(s)-len(suffix)]
	}
	return s
}

split_multi :: proc(s: string, substrs: []string, skip_empty := false, allocator := context.allocator) -> []string #no_bounds_check {
	if s == "" || len(substrs) <= 0 {
		return nil
	}

	sublen := len(substrs[0])

	for substr in substrs[1:] {
		sublen = min(sublen, len(substr))
	}

	shared := len(s) - sublen

	if shared <= 0 {
		return nil
	}

	// number, index, last
	n, i, l := 0, 0, 0

	// count results
	first_pass: for i <= shared {
		for substr in substrs {
			if s[i:i+sublen] == substr {
				if !skip_empty || i - l > 0 {
					n += 1
				}

				i += sublen
				l  = i

				continue first_pass
			}
		}

		_, skip := utf8.decode_rune_in_string(s[i:])
		i += skip
	}

	if !skip_empty || len(s) - l > 0 {
		n += 1
	}

	if n < 1 {
		// no results
		return nil
	}

	buf := make([]string, n, allocator)

	n, i, l = 0, 0, 0

	// slice results
	second_pass: for i <= shared {
		for substr in substrs {
			if s[i:i+sublen] == substr {
				if !skip_empty || i - l > 0 {
					buf[n] = s[l:i]
					n += 1
				}

				i += sublen
				l  = i

				continue second_pass
			}
		}

		_, skip := utf8.decode_rune_in_string(s[i:])
		i += skip
	}

	if !skip_empty || len(s) - l > 0 {
		buf[n] = s[l:]
	}

	return buf
}




split_multi_iterator :: proc(s: ^string, substrs: []string, skip_empty := false) -> (string, bool) #no_bounds_check {
	if s == nil || s^ == "" || len(substrs) <= 0 {
		return "", false
	}

	sublen := len(substrs[0])

	for substr in substrs[1:] {
		sublen = min(sublen, len(substr))
	}

	shared := len(s) - sublen

	if shared <= 0 {
		return "", false
	}

	// index, last
	i, l := 0, 0

	loop: for i <= shared {
		for substr in substrs {
			if s[i:i+sublen] == substr {
				if !skip_empty || i - l > 0 {
					res := s[l:i]
					s^ = s[i:]
					return res, true
				}

				i += sublen
				l  = i

				continue loop
			}
		}

		_, skip := utf8.decode_rune_in_string(s[i:])
		i += skip
	}

	if !skip_empty || len(s) - l > 0 {
		res := s[l:]
		s^ = s[len(s):]
		return res, true
	}

	return "", false
}






// scrub scruvs invalid utf-8 characters and replaces them with the replacement string
// Adjacent invalid bytes are only replaced once
scrub :: proc(s: string, replacement: string, allocator := context.allocator) -> string {
	str := s
	b: Builder
	init_builder(&b, 0, len(s), allocator)

	has_error := false
	cursor := 0
	origin := str

	for len(str) > 0 {
		r, w := utf8.decode_rune_in_string(str)

		if r == utf8.RUNE_ERROR {
			if !has_error {
				has_error = true
				write_string(&b, origin[:cursor])
			}
		} else if has_error {
			has_error = false
			write_string(&b, replacement)

			origin = origin[cursor:]
			cursor = 0
		}

		cursor += w
		str = str[w:]
	}

	return to_string(b)
}


reverse :: proc(s: string, allocator := context.allocator) -> string {
	str := s
	n := len(str)
	buf := make([]byte, n)
	i := n

	for len(str) > 0 {
		_, w := utf8.decode_rune_in_string(str)
		i -= w
		copy(buf[i:], str[:w])
		str = str[w:]
	}
	return string(buf)
}

expand_tabs :: proc(s: string, tab_size: int, allocator := context.allocator) -> string {
	if tab_size <= 0 {
		panic("tab size must be positive")
	}


	if s == "" {
		return ""
	}

	b: Builder
	init_builder(&b, allocator)
	writer := to_writer(&b)
	str := s
	column: int

	for len(str) > 0 {
		r, w := utf8.decode_rune_in_string(str)

		if r == '\t' {
			expand := tab_size - column%tab_size

			for i := 0; i < expand; i += 1 {
				io.write_byte(writer, ' ')
			}

			column += expand
		} else {
			if r == '\n' {
				column = 0
			} else {
				column += w
			}

			io.write_rune(writer, r)
		}

		str = str[w:]
	}

	return to_string(b)
}


partition :: proc(str, sep: string) -> (head, match, tail: string) {
	i := index(str, sep)
	if i == -1 {
		head = str
		return
	}

	head = str[:i]
	match = str[i:i+len(sep)]
	tail = str[i+len(sep):]
	return
}

center_justify :: centre_justify // NOTE(bill): Because Americans exist

// centre_justify returns a string with a pad string at boths sides if the str's rune length is smaller than length
centre_justify :: proc(str: string, length: int, pad: string, allocator := context.allocator) -> string {
	n := rune_count(str)
	if n >= length || pad == "" {
		return clone(str, allocator)
	}

	remains := length-1
	pad_len := rune_count(pad)

	b: Builder
	init_builder(&b, allocator)
	grow_builder(&b, len(str) + (remains/pad_len + 1)*len(pad))

	w := to_writer(&b)

	write_pad_string(w, pad, pad_len, remains/2)
	io.write_string(w, str)
	write_pad_string(w, pad, pad_len, (remains+1)/2)

	return to_string(b)
}

// left_justify returns a string with a pad string at left side if the str's rune length is smaller than length
left_justify :: proc(str: string, length: int, pad: string, allocator := context.allocator) -> string {
	n := rune_count(str)
	if n >= length || pad == "" {
		return clone(str, allocator)
	}

	remains := length-1
	pad_len := rune_count(pad)

	b: Builder
	init_builder(&b, allocator)
	grow_builder(&b, len(str) + (remains/pad_len + 1)*len(pad))

	w := to_writer(&b)

	io.write_string(w, str)
	write_pad_string(w, pad, pad_len, remains)

	return to_string(b)
}

// right_justify returns a string with a pad string at right side if the str's rune length is smaller than length
right_justify :: proc(str: string, length: int, pad: string, allocator := context.allocator) -> string {
	n := rune_count(str)
	if n >= length || pad == "" {
		return clone(str, allocator)
	}

	remains := length-1
	pad_len := rune_count(pad)

	b: Builder
	init_builder(&b, allocator)
	grow_builder(&b, len(str) + (remains/pad_len + 1)*len(pad))

	w := to_writer(&b)

	write_pad_string(w, pad, pad_len, remains)
	io.write_string(w, str)

	return to_string(b)
}




@private
write_pad_string :: proc(w: io.Writer, pad: string, pad_len, remains: int) {
	repeats := remains / pad_len

	for i := 0; i < repeats; i += 1 {
		io.write_string(w, pad)
	}

	n := remains % pad_len
	p := pad

	for i := 0; i < n; i += 1 {
		r, width := utf8.decode_rune_in_string(p)
		io.write_rune(w, r)
		p = p[width:]
	}
}


// fields splits the string s around each instance of one or more consecutive white space character, defined by unicode.is_space
// returning a slice of substrings of s or an empty slice if s only contains white space
fields :: proc(s: string, allocator := context.allocator) -> []string #no_bounds_check {
	n := 0
	was_space := 1
	set_bits := u8(0)

	// check to see
	for i in 0..<len(s) {
		r := s[i]
		set_bits |= r
		is_space := int(_ascii_space[r])
		n += was_space & ~is_space
		was_space = is_space
	}

	if set_bits >= utf8.RUNE_SELF {
		return fields_proc(s, unicode.is_space, allocator)
	}

	if n == 0 {
		return nil
	}

	a := make([]string, n, allocator)
	na := 0
	field_start := 0
	i := 0
	for i < len(s) && _ascii_space[s[i]] {
		i += 1
	}
	field_start = i
	for i < len(s) {
		if !_ascii_space[s[i]] {
			i += 1
			continue
		}
		a[na] = s[field_start : i]
		na += 1
		i += 1
		for i < len(s) && _ascii_space[s[i]] {
			i += 1
		}
		field_start = i
	}
	if field_start < len(s) {
		a[na] = s[field_start:]
	}
	return a
}


// fields_proc splits the string s at each run of unicode code points `ch` satisfying f(ch)
// returns a slice of substrings of s
// If all code points in s satisfy f(ch) or string is empty, an empty slice is returned
//
// fields_proc makes no guarantee about the order in which it calls f(ch)
// it assumes that `f` always returns the same value for a given ch
fields_proc :: proc(s: string, f: proc(rune) -> bool, allocator := context.allocator) -> []string #no_bounds_check {
	substrings := make([dynamic]string, 0, 32, allocator)

	start, end := -1, -1
	for r, offset in s {
		end = offset
		if f(r) {
			if start >= 0 {
				append(&substrings, s[start : end])
				// -1 could be used, but just speed it up through bitwise not
				// gotta love 2's complement
				start = ~start
			}
		} else {
			if start < 0 {
				start = end
			}
		}
	}

	if start >= 0 {
		append(&substrings, s[start : len(s)])
	}

	return substrings[:]
}
