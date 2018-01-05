// Copyright (C) 2007 by Rui Maciel
// rui.maciel@gmail.com
//
// This program is free software; you can redistribute it and/or modify it under the terms of the
// GNU Library General Public License as published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
#include <slib.h>
#include <tv.h>
#pragma hdrstop
#include <json.h>

enum LEX_VALUE {
	LEX_MORE = 0,
	LEX_INVALID_CHARACTER,
	LEX_TRUE,
	LEX_FALSE,
	LEX_NULL,
	LEX_BEGIN_OBJECT,
	LEX_END_OBJECT,
	LEX_BEGIN_ARRAY,
	LEX_END_ARRAY,
	LEX_NAME_SEPARATOR,
	LEX_VALUE_SEPARATOR,
	LEX_STRING,
	LEX_NUMBER,
	LEX_ERROR,
	LEX_MEMORY
};

/* rc_string part */

#define RSTRING_INCSTEP 5
#define RSTRING_DEFAULT 8

enum rstring_code {
	RS_MEMORY,
	RS_OK = 1,
	RS_UNKNOWN
};

//typedef enum rui_string_error_codes rstring_code;

static RcString * FASTCALL rcs_create(size_t length)
{
	RcString * rcs = (RcString *)SAlloc::M(sizeof(RcString)); // allocates memory for a struct RcString
	if(rcs) {
		rcs->max = length;
		rcs->length = 0;
		THROW(rcs->P_Text = (char *)SAlloc::M((rcs->max + 1) * sizeof(char)));
		rcs->P_Text[0] = '\0';
	}
	CATCH
		ZFREE(rcs);
	ENDCATCH
	return rcs;
}

static void FASTCALL rcs_free(RcString ** rcs)
{
	assert(rcs);
	if(*rcs) {
		ZFREE((*rcs)->P_Text);
		ZFREE(*rcs);
	}
}

static rstring_code FASTCALL rcs_resize(RcString * rcs, size_t length)
{
	assert(rcs);
	char * temp = (char *)SAlloc::R(rcs->P_Text, sizeof(char) * (length + 1)); // length plus '\0'
	if(temp == NULL) {
		SAlloc::F(rcs);
		return RS_MEMORY;
	}
	else {
		rcs->P_Text = temp;
		rcs->max = length;
		rcs->P_Text[rcs->max] = '\0';
		return RS_OK;
	}
}

rstring_code rcs_catcs(RcString * pre, const char * pos, const size_t length)
{
	assert(pre);
	assert(pos);
	if(pre->max < (pre->length + length))
		if(rcs_resize(pre, pre->length + length + RSTRING_INCSTEP) != RS_OK)
			return RS_MEMORY;
	strncpy(pre->P_Text + pre->length, pos, length);
	pre->P_Text[pre->length + length] = '\0';
	pre->length += length;
	return RS_OK;
}

static rstring_code FASTCALL rcs_catc(RcString * pre, const char c)
{
	assert(pre);
	if(pre->max <= pre->length)
		if(rcs_resize(pre, pre->max + RSTRING_INCSTEP) != RS_OK)
			return RS_MEMORY;
	pre->P_Text[pre->length] = c;
	pre->length++;
	pre->P_Text[pre->length] = '\0';
	return RS_OK;
}

static char * FASTCALL rcs_unwrap(RcString * rcs)
{
	assert(rcs);
	char * out = (rcs->P_Text == NULL) ? 0 : (char *)SAlloc::R(rcs->P_Text, sizeof(char) * (strlen(rcs->P_Text) + 1));
	SAlloc::F(rcs);
	return out;
}

/*static size_t FASTCALL rcs_length(RcString * rcs)
{
	// @todo account for UTF8
	assert(rcs);
	return rcs->length;
}*/

// end of rc_string part

json_t::json_t(/*enum json_value_type*/int aType) : Type(aType), P_Next(0), P_Previous(0), P_Parent(0), P_Child(0), P_ChildEnd(0)
{
	//P_Text = 0;
}

json_t::~json_t()
{
	//ZFREE(P_Text);
}

enum json_error json_stream_parse(FILE * file, json_t ** document)
{
	char buffer[1024];	/* hard-coded value */
	enum json_error error = JSON_INCOMPLETE_DOCUMENT;
	json_parsing_info state;
	assert(file); // must be an open stream 
	assert(document); // must be a valid pointer reference 
	assert(*document == NULL); // only accepts a null json_t pointer, to avoid memory leaks 
	json_jpi_init(&state);	/* initializes the json_parsing_info object */
	while((error == JSON_WAITING_FOR_EOF) || (error == JSON_INCOMPLETE_DOCUMENT)) {
		if(fgets(buffer, 1024, file)) {
			switch(error = json_parse_fragment(&state, buffer)) {
				case JSON_OK:
				case JSON_WAITING_FOR_EOF:
				case JSON_INCOMPLETE_DOCUMENT:
					break;
				default:
					json_free_value(&state.cursor);
					return error;
					break;
			}
		}
		else {
			if(error == JSON_WAITING_FOR_EOF)
				error = JSON_OK;
			else
				error = JSON_UNKNOWN_PROBLEM; // @todo refine this error code
		}
	}
	if(error == JSON_OK)
		*document = state.cursor;
	return error;
}

#if 0 // {
json_t * FASTCALL json_new_value(/*json_value_type*/int type)
{
	//json_t * p_new_object = (json_t *)SAlloc::M(sizeof(json_t));
	return new json_t(type);
}
#endif // } 0

json_t * json_new_string(const char * pText)
{
	assert(pText);
	json_t * p_new_object = new json_t(json_t::tSTRING);
	if(p_new_object)
		p_new_object->Text = pText;
	return p_new_object;
}

json_t * json_new_number(const char * pText)
{
	assert(pText);
	json_t * p_new_object = new json_t(json_t::tNUMBER);
	if(p_new_object)
		p_new_object->Text = pText;
	return p_new_object;
}

// json_t * json_new_object() { return new json_t(json_t::tOBJECT); }
// json_t * json_new_array() { return new json_t(json_t::tARRAY); }
// json_t * json_new_null() { return new json_t(json_t::tNULL); }
// json_t * json_new_true() { return new json_t(json_t::tTRUE); }
// json_t * json_new_false() { return new json_t(json_t::tFALSE); }

/*static void FASTCALL intern_json_free_value(json_t ** ppValue)
{
	assert(ppValue);
	assert((*ppValue));
	assert((*ppValue)->P_Child == NULL);
	// fixing sibling linked list connections
	if((*ppValue)->P_Previous && (*ppValue)->P_Next) {
		(*ppValue)->P_Previous->P_Next = (*ppValue)->P_Next;
		(*ppValue)->P_Next->P_Previous = (*ppValue)->P_Previous;
	}
	else {
		if((*ppValue)->P_Previous)
			(*ppValue)->P_Previous->P_Next = NULL;
		if((*ppValue)->P_Next)
			(*ppValue)->P_Next->P_Previous = NULL;
	}
	// fixing parent node connections
	if((*ppValue)->P_Parent) {
		// fix the tree connection to the first node in the children's list
		if((*ppValue)->P_Parent->P_Child == (*ppValue)) {
			if((*ppValue)->P_Next)
				(*ppValue)->P_Parent->P_Child = (*ppValue)->P_Next; // the parent node always points to the first node in the children linked list
			else
				(*ppValue)->P_Parent->P_Child = NULL;
		}
		// fix the tree connection to the last node in the children's list
		if((*ppValue)->P_Parent->P_ChildEnd == (*ppValue)) {
			if((*ppValue)->P_Previous)
				(*ppValue)->P_Parent->P_ChildEnd = (*ppValue)->P_Previous; // the parent node always points to the last node in the children linked list
			else
				(*ppValue)->P_Parent->P_ChildEnd = NULL;
		}
	}
	// finally, freeing the memory allocated for this value
	SAlloc::F((*ppValue)->P_Text);
	ZFREE(*ppValue); // the json value
}*/

void json_free_value(json_t ** ppValue)
{
	if(ppValue && *ppValue) {
		json_t * p_cursor = *ppValue;
		//assert(value);
		//assert(*value);
		while(*ppValue) {
			if(p_cursor->P_Child) {
				p_cursor = p_cursor->P_Child;
			}
			else {
				json_t * p_parent = p_cursor->P_Parent;
				if(p_cursor == *ppValue)
					*ppValue = NULL;
				//intern_json_free_value(&cursor);
				//static void FASTCALL intern_json_free_value(json_t ** ppValue)
				{
					assert(p_cursor);
					assert(p_cursor->P_Child == NULL);
					// fixing sibling linked list connections
					if(p_cursor->P_Previous && p_cursor->P_Next) {
						p_cursor->P_Previous->P_Next = p_cursor->P_Next;
						p_cursor->P_Next->P_Previous = p_cursor->P_Previous;
					}
					else {
						if(p_cursor->P_Previous)
							p_cursor->P_Previous->P_Next = 0;
						if(p_cursor->P_Next)
							p_cursor->P_Next->P_Previous = 0;
					}
					// fixing parent node connections
					if(p_cursor->P_Parent) {
						// fix the tree connection to the first node in the children's list
						if(p_cursor->P_Parent->P_Child == p_cursor)
							p_cursor->P_Parent->P_Child = p_cursor->P_Next; // the parent node always points to the first node in the children linked list
						// fix the tree connection to the last node in the children's list
						if(p_cursor->P_Parent->P_ChildEnd == p_cursor)
							p_cursor->P_Parent->P_ChildEnd = p_cursor->P_Previous; // the parent node always points to the last node in the children linked list
					}
				}
				delete p_cursor;
				p_cursor = p_parent;
			}
		}
	}
}

enum json_error FASTCALL json_insert_child(json_t * pParent, json_t * pChild)
{
	// @todo change the child list from FIFO to LIFO, in order to get rid of the child_end pointer
	assert(pParent); // the parent must exist
	assert(pChild); // the child must exist
	assert(pParent != pChild); // parent and child must not be the same. if they are, it will enter an infinite loop
	// enforce tree structure correctness
	switch(pParent->Type) {
		case json_t::tSTRING:
			// a string accepts every JSON type as a child value
			// therefore, the sanity check must be performed on the child node
			switch(pChild->Type) {
				case json_t::tSTRING:
				case json_t::tNUMBER:
				case json_t::tTRUE:
				case json_t::tFALSE:
				case json_t::tNULL:
					if(pChild->P_Child)
						return JSON_BAD_TREE_STRUCTURE;
					break;
				case json_t::tOBJECT:
				case json_t::tARRAY:
					break;
				default:
					return JSON_BAD_TREE_STRUCTURE; // this part should never be reached
					break;
			}
			break;
		case json_t::tOBJECT: // JSON objects may only accept JSON string objects which already have child nodes of their own
			if(pChild->Type != json_t::tSTRING)
				return JSON_BAD_TREE_STRUCTURE;
			break;
		case json_t::tARRAY:
			switch(pChild->Type) {
				case json_t::tSTRING:
				case json_t::tTRUE:
				case json_t::tFALSE:
				case json_t::tNULL:
				case json_t::tNUMBER:
					if(pChild->P_Child)
						return JSON_BAD_TREE_STRUCTURE;
					break;
				case json_t::tOBJECT:
				case json_t::tARRAY:
					break;
				default:
					return JSON_BAD_TREE_STRUCTURE;
			}
			break;
		default:
			return JSON_BAD_TREE_STRUCTURE;
	}
	pChild->P_Parent = pParent;
	if(pParent->P_Child) {
		pChild->P_Previous = pParent->P_ChildEnd;
		pParent->P_ChildEnd->P_Next = pChild;
		pParent->P_ChildEnd = pChild;
	}
	else {
		pParent->P_Child = pChild;
		pParent->P_ChildEnd = pChild;
	}
	return JSON_OK;
}

enum json_error json_insert_pair_into_object(json_t * pParent, const char * pTextLabel, json_t * pValue)
{
	enum     json_error error;
	// verify if the parameters are valid
	assert(pParent);
	assert(pTextLabel);
	assert(pValue);
	assert(pParent != pValue);
	// enforce type coherence
	assert(pParent->Type == json_t::tOBJECT);
	// create label json_value
	json_t * label = json_new_string (pTextLabel);
	if(label == NULL)
		error = JSON_MEMORY;
	else {
		// insert value and check for error
		error = json_insert_child(label, pValue);
		if(error == JSON_OK)
			error = json_insert_child(pParent, label); // insert value and check for error
	}
	return error;
}

enum json_error json_tree_to_string(json_t * pRoot, char ** ppText)
{
	assert(pRoot);
	assert(ppText);
	json_t * cursor = pRoot;
	// set up the output and temporary rwstrings
	RcString * output = rcs_create(RSTRING_DEFAULT);
	// start the convoluted fun
state1: // open value
	if(cursor->P_Previous && cursor != pRoot) { // if cursor is children and not root than it is a followup sibling
		// append comma
		rcs_catc(output, ',');
	}
	switch(cursor->Type) {
		case json_t::tSTRING:
			// append the "text"\0, which means 1 + wcslen(cursor->text) + 1 + 1
			// set the new output size
			rcs_catc(output, '\"');
			rcs_catcs(output, cursor->Text, cursor->Text.Len());
			rcs_catc(output, '\"');
			if(cursor->P_Parent) {
				if(cursor->P_Parent->Type == json_t::tOBJECT)	{ // cursor is label in label:value pair
					// error checking: if parent is object and cursor is string then cursor must have a single child
					if(cursor->P_Child)
						rcs_catc(output, ':');
					else {
						// malformed document tree: label without value in label:value pair
						rcs_free(&output);
						ppText = NULL;
						return JSON_BAD_TREE_STRUCTURE;
					}
				}
			}
			else {	// does not have a parent
				if(cursor->P_Child) // is root label in label:value pair
					rcs_catc(output, ':');
				else {
					// malformed document tree: label without value in label:value pair
					rcs_free(&output);
					ppText = NULL;
					return JSON_BAD_TREE_STRUCTURE;
				}
			}
			break;
		case json_t::tNUMBER:
			// must not have any children
			// set the new size
			rcs_catcs(output, cursor->Text, cursor->Text.Len());
			goto state2; // close value
			break;
		case json_t::tOBJECT:
			rcs_catc(output, '{');
			if(cursor->P_Child) {
				cursor = cursor->P_Child;
				goto state1; // open value
			}
			else
				goto state2; // close value
			break;
		case json_t::tARRAY:
			rcs_catc(output, '[');
			if(cursor->P_Child) {
				cursor = cursor->P_Child;
				goto state1;
			}
			else
				goto state2; // close value
			break;
		case json_t::tTRUE:
			// must not have any children
			rcs_catcs(output, "true", 4);
			goto state2; // close value
			break;
		case json_t::tFALSE:
			// must not have any children
			rcs_catcs(output, "false", 5);
			goto state2; // close value
			break;
		case json_t::tNULL:
			// must not have any children
			rcs_catcs(output, "null", 4);
			goto state2; // close value
			break;
		default:
			goto error;
	}
	if(cursor->P_Child) {
		cursor = cursor->P_Child;
		goto state1; // open value */
	}
	else // does not have any children
		goto state2; // close value
state2: // close value
	switch(cursor->Type) {
		case json_t::tOBJECT: rcs_catc(output, '}'); break;
		case json_t::tARRAY:  rcs_catc(output, ']'); break;
		case json_t::tSTRING: break;
		case json_t::tNUMBER: break;
		case json_t::tTRUE:   break;
		case json_t::tFALSE:  break;
		case json_t::tNULL:   break;
		default: goto error;
	}
	if(!cursor->P_Parent || cursor == pRoot)
		goto end;
	else if(cursor->P_Next) {
		cursor = cursor->P_Next;
		goto state1; // open value
	}
	else {
		cursor = cursor->P_Parent;
		goto state2; // close value
	}
error:
	rcs_free(&output);
	return JSON_UNKNOWN_PROBLEM;
end:
	*ppText = rcs_unwrap(output);
	return JSON_OK;
}

enum json_error json_stream_output(json_t * root, SString & rBuf)
{
	assert(root);
	//assert(file); // the file stream must be opened
	json_t * cursor = root;
	// set up the output and temporary rwstrings

// start the convoluted fun
state1: // open value
	if(cursor->P_Previous && cursor != root) { // if cursor is children and not root than it is a followup sibling
		// append comma
		//fprintf(file, ",");
		rBuf.Comma();
	}
	switch(cursor->Type) {
		case json_t::tSTRING:
			// append the "text"\0, which means 1 + wcslen(cursor->text) + 1 + 1
			// set the new output size
			//fprintf(file, "\"%s\"", cursor->P_Text);
			rBuf.CatQStr(cursor->Text);
			if(cursor->P_Parent) {
				if(cursor->P_Parent->Type == json_t::tOBJECT) { // cursor is label in label:value pair
					// error checking: if parent is object and cursor is string then cursor must have a single child
					if(cursor->P_Child) {
						//fprintf(file, ":");
						rBuf.CatChar(':');
					}
					else
						// malformed document tree: label without value in label:value pair
						return JSON_BAD_TREE_STRUCTURE;
				}
			}
			else { // does not have a parent
				if(cursor->P_Child) { // is root label in label:value pair
					//fprintf(file, ":");
					rBuf.CatChar(':');
				}
				else // malformed document tree: label without value in label:value pair
					return JSON_BAD_TREE_STRUCTURE;
			}
			break;
		case json_t::tNUMBER:
			// must not have any children
			// set the new size
			//fprintf(file, "%s", cursor->P_Text);
			rBuf.Cat(cursor->Text);
			goto state2; // close value
			break;
		case json_t::tOBJECT:
			//fprintf(file, "{");
			rBuf.CatChar('{');
			if(cursor->P_Child) {
				cursor = cursor->P_Child;
				goto state1; // open value
			}
			else
				goto state2; // close value
			break;
		case json_t::tARRAY:
			//fprintf(file, "[");
			rBuf.CatChar('[');
			if(cursor->P_Child) {
				cursor = cursor->P_Child;
				goto state1;
			}
			else
				goto state2; // close value
			break;
		case json_t::tTRUE: // must not have any children
			//fprintf(file, "true");
			rBuf.Cat("true");
			goto state2; // close value
			break;
		case json_t::tFALSE: // must not have any children
			//fprintf(file, "false");
			rBuf.Cat("false");
			goto state2; // close value
			break;
		case json_t::tNULL: // must not have any children
			//fprintf(file, "null");
			rBuf.Cat("null");
			goto state2; // close value
			break;
		default:
			goto error;
	}
	if(cursor->P_Child) {
		cursor = cursor->P_Child;
		goto state1; // open value
	}
	else
		// does not have any children
		goto state2; // close value
state2: // close value
	switch(cursor->Type) {
		case json_t::tOBJECT: 
			//fprintf(file, "}"); 
			rBuf.CatChar('}');
			break;
		case json_t::tARRAY:  
			//fprintf(file, "]"); 
			rBuf.CatChar(']');
			break;
		case json_t::tSTRING: break;
		case json_t::tNUMBER: break;
		case json_t::tTRUE:   break;
		case json_t::tFALSE:  break;
		case json_t::tNULL:   break;
		default:          goto error;
	}
	if(!cursor->P_Parent || cursor == root)
		goto end;
	else if(cursor->P_Next) {
		cursor = cursor->P_Next;
		goto state1; // open value
	}
	else {
		cursor = cursor->P_Parent;
		goto state2; // close value
	}
error:
	return JSON_UNKNOWN_PROBLEM;
end:
	//fprintf(file, "\n");
	rBuf.CR();
	return JSON_OK;
}

void json_strip_white_spaces(char * text)
{
	assert(text);
	size_t in = 0;
	size_t out = 0;
	size_t length = strlen(text);
	int    state = 0; // possible states: 0 -> document, 1 -> inside a string
	while(in < length) {
		switch(text[in]) {
			case '\x20': // space
			case '\x09': // horizontal tab
			case '\x0A': // line feed or new line
			case '\x0D': // Carriage return
				if(state == 1)
					text[out++] = text[in];
				break;
			case '\"':
				switch(state) {
					case 0: // not inside a JSON string
						state = 1;
						break;
					case 1: // inside a JSON string
						if(text[in - 1] != '\\')
							state = 0;
						break;
					default:
						assert(0);
				}
				text[out++] = text[in];
				break;
			default:
				text[out++] = text[in];
		}
		++in;
	}
	text[out] = '\0';
}

#if 0 // {
char * json_format_string(const char * text)
{
	size_t pos = 0;
	uint indentation = 0; // the current indentation level
	uint i; // loop iterator variable
	char loop;
	size_t text_length = strlen(text);
	RcString * p_output = rcs_create(text_length);
	while(pos < text_length) {
		switch(text[pos]) {
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D': // JSON insignificant white spaces
				pos++;
				break;
			case '{':
				indentation++;
				rcs_catcs(p_output, "{\n", 2);
				for(i = 0; i < indentation; i++)
					rcs_catc(p_output, '\t');
				pos++;
				break;
			case '}':
				indentation--;
				rcs_catc(p_output, '\n');
				for(i = 0; i < indentation; i++)
					rcs_catc(p_output, '\t');
				rcs_catc(p_output, '}');
				pos++;
				break;
			case ':':
				rcs_catcs(p_output, ": ", 2);
				pos++;
				break;
			case ',':
				rcs_catcs(p_output, ",\n", 2);
				for(i = 0; i < indentation; i++)
					rcs_catc(p_output, '\t');
				pos++;
				break;
			case '\"':	/* open string */
				rcs_catc(p_output, text[pos]);
				pos++;
				loop = 1; // inner string loop trigger is enabled
				while(loop) {
					if(text[pos] == '\\') { // escaped sequence
						rcs_catc(p_output, '\\');
						pos++;
						if(text[pos] == '\"') { // don't consider a \" escaped sequence as an end of string
							rcs_catc(p_output, '\"');
							pos++;
						}
					}
					else if(text[pos] == '\"') // reached end of string
						loop = 0;
					rcs_catc(p_output, text[pos]);
					pos++;
					if(pos >= text_length)
						loop = 0;
				}
				break;

			default:
				rcs_catc(p_output, text[pos]);
				pos++;
				break;
		}
	}
	return rcs_unwrap(p_output);
}
#endif // } 0

int json_format_string(const char * pText, SString & rBuf)
{
	int    ok = 1;
	size_t pos = 0;
	uint indentation = 0; // the current indentation level
	char loop;
	size_t text_length = sstrlen(pText);
	//RcString * p_output = rcs_create(text_length);
	rBuf.Z();
	while(pos < text_length) {
		switch(pText[pos]) {
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D': // JSON insignificant white spaces
				pos++;
				break;
			case '{':
				indentation++;
				rBuf.CatChar('{').CR();
				rBuf.Tab(indentation);
				pos++;
				break;
			case '}':
				THROW(indentation > 0);
				indentation--;
				rBuf.CR();
				rBuf.Tab(indentation);
				rBuf.CatChar('}');
				pos++;
				break;
			case ':':
				rBuf.CatDiv(':', 2);
				pos++;
				break;
			case ',':
				rBuf.Comma().CR();
				rBuf.Tab(indentation);
				pos++;
				break;
			case '\"':	// open string 
				rBuf.CatChar(pText[pos]);
				pos++;
				loop = 1; // inner string loop trigger is enabled
				while(loop) {
					if(pText[pos] == '\\') { // escaped sequence
						rBuf.CatChar('\\');
						pos++;
						if(pText[pos] == '\"') { // don't consider a \" escaped sequence as an end of string
							rBuf.CatChar('\"');
							pos++;
						}
					}
					else if(pText[pos] == '\"') // reached end of string
						loop = 0;
					rBuf.CatChar(pText[pos]);
					pos++;
					if(pos >= text_length)
						loop = 0;
				}
				break;
			default:
				rBuf.CatChar(pText[pos]);
				pos++;
				break;
		}
	}
	CATCHZOK
	//return rcs_unwrap(p_output);
	return ok;
}

#if 0 // @v9.7.10 @obsolte {
char * json_escape(const char * pText)
{
	RcString * output;
	size_t i, length;
	char buffer[6];
	// check if pre-conditions are met
	assert(pText);
	// defining the temporary variables
	length = strlen(pText);
	output = rcs_create(length);
	if(!output)
		return NULL;
	for(i = 0; i < length; i++) {
		if(pText[i] == '\\')
			rcs_catcs(output, "\\\\", 2);
		else if(pText[i] == '\"')
			rcs_catcs(output, "\\\"", 2);
		else if(pText[i] == '/')
			rcs_catcs(output, "\\/", 2);
		else if(pText[i] == '\b')
			rcs_catcs(output, "\\b", 2);
		else if(pText[i] == '\f')
			rcs_catcs(output, "\\f", 2);
		else if(pText[i] == '\n')
			rcs_catcs(output, "\\n", 2);
		else if(pText[i] == '\r')
			rcs_catcs(output, "\\r", 2);
		else if(pText[i] == '\t')
			rcs_catcs(output, "\\t", 2);
		else if(pText[i] < 0) // non-BMP character
			rcs_catc(output, pText[i]);
		else if(pText[i] < 0x20) {
			sprintf(buffer, "\\u%4.4x", pText[i]);
			rcs_catcs(output, buffer, 6);
		}
		else
			rcs_catc(output, pText[i]);
	}
	return rcs_unwrap(output);
}

char * json_unescape(char * text)
{
	assert(text);
	char * result = (char *)SAlloc::M(strlen(text) + 1);
	size_t r; // read cursor
	size_t w; // write cursor
	for(r = w = 0; text[r]; r++) {
		switch(text[r]) {
			case '\\':
				switch(text[++r]) {
					case '\"':
					case '\\':
					case '/': result[w++] = text[r]; break; // literal translation
					case 'b': result[w++] = '\b'; break;
					case 'f': result[w++] = '\f'; break;
					case 'n': result[w++] = '\n'; break;
					case 'r': result[w++] = '\r'; break;
					case 't': result[w++] = '\t'; break;
					case 'u':
					{
						char buf[5];
						__int64 unicode;
						buf[0] = text[++r];
						buf[1] = text[++r];
						buf[2] = text[++r];
						buf[3] = text[++r];
						buf[4] = '\0';
						unicode = strtol(buf, NULL, 16);
						if(unicode < 0x80)
							result[w++] = (char)unicode; // ASCII: map to UTF-8 literally
						else if(unicode < 0x800) {
							/* two-byte-encoding */
							char one = (char)0xC0; /* 110 00000 */
							char two = (char)0x80; /* 10 000000 */
							two += (char)(unicode & 0x3F);
							unicode >>= 6;
							one += (char)(unicode & 0x1F);
							result[w++] = one;
							result[w++] = two;
						}
						else if(unicode < 0x10000) {
							if(unicode < 0xD800 || 0xDBFF < unicode) {
								/* three-byte-encoding */
								char one = (char)0xE0;   /* 1110 0000 */
								char two = (char)0x80;   /* 10 000000 */
								char three = (char)0x80; /* 10 000000 */
								three += (char)(unicode & 0x3F);
								unicode >>= 6;
								two += (char)(unicode & 0x3F);
								unicode >>= 6;
								one += (char)(unicode & 0xF);

								result[w++] = one;
								result[w++] = two;
								result[w++] = three;
							}
							else {
								// unicode is a UTF-16 high surrogate, continue with the low surrogate
								__int64 high_surrogate = unicode;	// 110110 00;00000000
								__int64 low_surrogate;
								char one   = (char)0xF0; // 11110 000
								char two   = (char)0x80; // 10 000000
								char three = (char)0x80; // 10 000000
								char four  = (char)0x80; // 10 000000
								if(text[++r] != '\\')
									break;
								if(text[++r] != 'u')
									break;

								buf[0] = text[++r];
								buf[1] = text[++r];
								buf[2] = text[++r];
								buf[3] = text[++r];

								low_surrogate = strtol(buf, NULL, 16); /* 110111 00;00000000 */

								// strip surrogate markers
								high_surrogate -= 0xD800; // 11011000;00000000
								low_surrogate -= 0xDC00; // 11011100;00000000

								unicode = (high_surrogate << 10) + (low_surrogate) + 0x10000;

								// now encode into four-byte UTF-8 (as we are larger than 0x10000)
								four += (char)(unicode & 0x3F);
								unicode >>= 6;
								three += (char)(unicode & 0x3F);
								unicode >>= 6;
								two += (char)(unicode & 0x3F);
								unicode >>= 6;
								one += (char)(unicode & 0x7);

								result[w++] = one;
								result[w++] = two;
								result[w++] = three;
								result[w++] = four;
							}
						}
						/*else
							fprintf(stderr, "JSON: unsupported unicode value: 0x%lX\n", (ulong)unicode);*/
						break;
					}
					default:
						assert(0);
						break;
				}
				break;
			default:
				result[w++] = text[r];
				break;
		}
	}
	result[w] = '\0';
	return result;
}
#endif // } 0 @v9.7.10 @obsolte

void json_jpi_init(json_parsing_info * jpi)
{
	assert(jpi);
	jpi->state = 0;
	jpi->lex_state = 0;
	jpi->lex_text = NULL;
	jpi->p = NULL;
	jpi->cursor = NULL;
	jpi->string_length_limit_reached = 0;
}

int lexer(const char * pBuffer, char ** p, uint * state, RcString ** text)
{
	assert(pBuffer);
	assert(p);
	assert(state);
	assert(text);
	if(*p == NULL)
		*p = (char *)pBuffer;
	while(**p != '\0') {
		switch(*state) {
			case 0:	/* Root document */
				switch(*(*p)++) {
					case '\x20':	/* space */
					case '\x09':	/* horizontal tab */
					case '\x0A':	/* line feed or new line */
					case '\x0D':	/* Carriage return */
						break;
					case '{': return LEX_BEGIN_OBJECT;
					case '}': return LEX_END_OBJECT;
					case '[': return LEX_BEGIN_ARRAY;
					case ']': return LEX_END_ARRAY;
					case ':': return LEX_NAME_SEPARATOR;
					case ',': return LEX_VALUE_SEPARATOR;
					case '\"':
						*text = rcs_create(RSTRING_DEFAULT);
						if(*text == NULL)
							return LEX_MEMORY;
						*state = 1;	/* inside a JSON string */
						break;
					case 't': *state =  7; break; // true: 1
					case 'f': *state = 10; break; // false: 1
					case 'n': *state = 14; break; // false: 1
					case '-':
						*text = rcs_create(RSTRING_DEFAULT);
						if(*text == NULL)
							return LEX_MEMORY;
						rcs_catc(*text, '-');
						*state = 17;	/* number: '0' */
						break;
					case '0':
						*text = rcs_create(RSTRING_DEFAULT);
						if(*text == NULL)
							return LEX_MEMORY;
						rcs_catc(*text, '0');
						*state = 18;	/* number: '0' */
						break;
					case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						*text = rcs_create(RSTRING_DEFAULT);
						if(*text == NULL)
							return LEX_MEMORY;
						rcs_catc(*text, *(*p - 1));
						*state = 19; // number: decimal followup
						break;
					default:
						return LEX_INVALID_CHARACTER;
				}
				break;
			case 1:	// inside a JSON string
				assert(*text);
				switch(**p) {
					case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
					case 10: // line feed
					case 11: case 12:
					case 13: // carriage return
					case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: 
					case 23: case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
						// ASCII control characters can only be present in a JSON string if they are escaped. If not then the document is invalid
						return LEX_INVALID_CHARACTER;
						break;
					case '\"':	// close JSON string
						// it is expected that, in the routine that calls this function, text is set to NULL
						*state = 0;
						++*p;
						return LEX_STRING;
						break;
					case '\\':
						rcs_catc(*text, '\\');
						*state = 2;	// inside a JSON string: start escape sequence
						break;
					default:
						rcs_catc(*text, **p);
						break;
				}
				++*p;
				break;
			case 2: // inside a JSON string: start escape sequence
				assert(*text);
				switch(**p) {
					case '\\':
					case '\"':
					case '/':
					case 'b':
					case 'f':
					case 'n':
					case 'r':
					case 't':
						rcs_catc(*text, **p);
						*state = 1;	// inside a JSON string
						break;
					case 'u':
						rcs_catc(*text, **p);
						*state = 3;	// inside a JSON string: escape unicode
						break;
					default:
						return LEX_INVALID_CHARACTER;
				}
				++*p;
				break;
			case 3: // inside a JSON string: escape unicode
				assert(*text);
				if((**p >= 'a') && (**p <= 'f')) {
					rcs_catc(*text, **p);
					*state = 4; // inside a JSON string: escape unicode
				}
				else if((**p >= 'A') && (**p <= 'F')) {
					rcs_catc(*text, **p);
					*state = 4;	// inside a JSON string: escape unicode
				}
				else if((**p >= '0') && (**p <= '9')) {
					rcs_catc(*text, **p);
					*state = 4;	// inside a JSON string: escape unicode
				}
				else
					return LEX_INVALID_CHARACTER;
				++*p;
				break;
			case 4:	// inside a JSON string: escape unicode
				assert(*text);
				if((**p >= 'a') && (**p <= 'f')) {
					rcs_catc(*text, **p);
					*state = 5;	// inside a JSON string: escape unicode
				}
				else if((**p >= 'A') && (**p <= 'F')) {
					rcs_catc(*text, **p);
					*state = 5;	// inside a JSON string: escape unicode
				}
				else if((**p >= '0') && (**p <= '9')) {
					rcs_catc(*text, **p);
					*state = 5;	// inside a JSON string: escape unicode
				}
				else
					return LEX_INVALID_CHARACTER;
				++*p;
				break;
			case 5:	// inside a JSON string: escape unicode
				assert(*text);
				if((**p >= 'a') && (**p <= 'f')) {
					rcs_catc(*text, **p);
					*state = 6;	// inside a JSON string: escape unicode
				}
				else if((**p >= 'A') && (**p <= 'F')) {
					rcs_catc(*text, **p);
					*state = 6;	// inside a JSON string: escape unicode
				}
				else if((**p >= '0') && (**p <= '9')) {
					rcs_catc(*text, **p);
					*state = 6;	// inside a JSON string: escape unicode
				}
				else
					return LEX_INVALID_CHARACTER;
				++*p;
				break;
			case 6:	// inside a JSON string: escape unicode
				assert(*text);
				if((**p >= 'a') && (**p <= 'f')) {
					rcs_catc(*text, **p);
					*state = 1;	/* inside a JSON string: escape unicode */
				}
				else if((**p >= 'A') && (**p <= 'F')) {
					rcs_catc(*text, **p);
					*state = 1;	/* inside a JSON string: escape unicode */
				}
				else if((**p >= '0') && (**p <= '9')) {
					rcs_catc(*text, **p);
					*state = 1;	/* inside a JSON string: escape unicode */
				}
				else
					return LEX_INVALID_CHARACTER;
				++*p;
				break;
			case 7:	/* true: 1 */
				switch(*(*p)++) {
					case 'r':
						*state = 8;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 8:	/* true: 2 */
				switch(*(*p)++) {
					case 'u':
						*state = 9;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 9:	/* true: 3 */
				switch(*(*p)++) {
					case 'e':
						*state = 0;
						return LEX_TRUE;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 10:	/* false: 1 */
				switch(*(*p)++) {
					case 'a':
						*state = 11;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 11:	/* false: 2 */
				switch(*(*p)++) {
					case 'l':
						*state = 12;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 12:	/* false: 3 */
				switch(*(*p)++) {
					case 's':
						*state = 13;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 13:	/* false: 4 */
				switch(*(*p)++) {
					case 'e':
						*state = 0;
						return LEX_FALSE;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 14:	/* null: 1 */
				switch(*(*p)++) {
					case 'u':
						*state = 15;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 15:	/* null: 2 */
				switch(*(*p)++) {
					case 'l':
						*state = 16;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 16:	/* null: 3 */
				switch(*(*p)++) {
				case 'l':
					*state = 0;
					return LEX_NULL;
					break;
				default:
					return LEX_INVALID_CHARACTER;
					break;
				}
				break;
			case 17: // number: minus sign
				assert(*text);
				switch(**p) {
					case '0':
						rcs_catc(*text, **p);
						++*p;
						*state = 18; // number: '0'
						break;
					case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						rcs_catc(*text, **p);
						++*p;
						*state = 19;	/* number: decimal followup */
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 18:	/* number: '0' */
				assert(*text);
				switch(**p) {
					case '\x20':	/* space */
					case '\x09':	/* horizontal tab */
					case '\x0A':	/* line feed or new line */
					case '\x0D':	/* Carriage return */
						++*p;
					case ']':
					case '}':
					case ',':
						*state = 0;
						return LEX_NUMBER;
						break;

					case '.':
						rcs_catc(*text, **p);
						++*p;
						*state = 20;	/* number: frac start */
						break;

					case 'e':
					case 'E':
						rcs_catc(*text, **p);
						++*p;
						*state = 22;	/* number: exp start */
						break;

					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 19:	/* number: int followup */
				assert(*text);
				switch(**p) {
					case '\x20':	/* space */
					case '\x09':	/* horizontal tab */
					case '\x0A':	/* line feed or new line */
					case '\x0D':	/* Carriage return */
						++*p;
					case ']':
					case '}':
					case ',':
						*state = 0;
						return LEX_NUMBER;
						break;
					case '.':
						rcs_catc(*text, **p);
						++*p;
						*state = 20;	/* number: frac start */
						break;
					case 'e':
					case 'E':
						rcs_catc(*text, **p);
						++*p;
						*state = 22;	/* number: exp start */
						break;
					case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						rcs_catc(*text, **p);
						++*p;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
				break;
			case 20: // number: frac start
				{
					assert(*text);
					switch(**p) {
						case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
							rcs_catc(*text, **p);
							++*p;
							*state = 21;	/* number: frac continue */
							break;
						default:
							return LEX_INVALID_CHARACTER;
							break;
					}
				}
				break;
			case 21: // number: frac continue
				{
					assert(*text);
					switch(**p) {
						case '\x20': // space
						case '\x09': // horizontal tab
						case '\x0A': // line feed or new line
						case '\x0D': // Carriage return
							++*p;
						case ']':
						case '}':
						case ',':
							*state = 0;
							return LEX_NUMBER;
							break;
						case 'e':
						case 'E':
							rcs_catc(*text, **p);
							++*p;
							*state = 22;	/* number: exp start */
							break;
						case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
							rcs_catc(*text, **p);
							++*p;
							break;
						default:
							return LEX_INVALID_CHARACTER;
							break;
					}
				}
				break;
			case 22: // number: exp start
			{
				assert(*text);
				switch(**p) {
					case '-':
					case '+':
						rcs_catc(*text, **p);
						++*p;
						*state = 23;	/* number: exp continue */
						break;
					case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						rcs_catc(*text, **p);
						++*p;
						*state = 24;	/* number: exp end */
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
			}
			break;
			case 23: // number: exp continue
			{
				assert(*text);
				switch(**p) {
					case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						rcs_catc(*text, **p);
						++*p;
						*state = 24;	/* number: exp end */
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
			}
			break;
			case 24: // number: exp end
			{
				assert(*text);
				switch(**p) {
					case '\x20':	/* space */
					case '\x09':	/* horizontal tab */
					case '\x0A':	/* line feed or new line */
					case '\x0D':	/* Carriage return */
						++*p;
					case ']':
					case '}':
					case ',':
						*state = 0;
						return LEX_NUMBER;
						break;
					case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						rcs_catc(*text, **p);
						++*p;
						break;
					default:
						return LEX_INVALID_CHARACTER;
						break;
				}
			}
			break;
			default:
				// fprintf(stderr, "JSON: *state missing: %d\n", *state);
				return LEX_INVALID_CHARACTER;
		}
	}
	*p = NULL;
	return LEX_MORE;
}

enum json_error json_parse_fragment(json_parsing_info *info, const char *buffer)
{
	json_t * p_temp = 0;
	assert(info);
	assert(buffer);
	info->p = (char *)buffer;
	while(*info->p != '\0') {
		switch(info->state) {
			case 0:	/* starting point */
				{
					switch(lexer(buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_BEGIN_OBJECT: info->state = 1; break; // begin object
						case LEX_BEGIN_ARRAY:  info->state = 7; break; // begin array
						case LEX_INVALID_CHARACTER: return JSON_MALFORMED_DOCUMENT; break;
						default:
							//fprintf(stderr, "JSON: state %d: defaulted\n", info->state);
							return JSON_MALFORMED_DOCUMENT;
							break;
					}
				}
				break;
			case 1:	/* open object */
				{
					if(info->cursor == NULL) {
						if((info->cursor = new json_t(json_t::tOBJECT)) == NULL) {
							return JSON_MEMORY;
						}
					}
					else {
						/* perform tree sanity check */
						assert((info->cursor->Type == json_t::tSTRING) || (info->cursor->Type == json_t::tARRAY));
						if((p_temp = new json_t(json_t::tOBJECT)) == NULL) {
							return JSON_MEMORY;
						}
						if(json_insert_child(info->cursor, p_temp) != JSON_OK) {
							return JSON_UNKNOWN_PROBLEM;
						}
						info->cursor = p_temp;
						p_temp = NULL;
					}
					info->state = 2; // just entered an object
				}
				break;
			case 2: // opened object
				{
					//
					// perform tree sanity checks
					//
					assert(info->cursor);
					assert(info->cursor->Type == json_t::tOBJECT);
					switch(lexer(buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_STRING:
							if((p_temp = new json_t(json_t::tSTRING)) == NULL)
								return JSON_MEMORY;
							//p_temp->P_Text = rcs_unwrap(info->lex_text);
							p_temp->AssignAllocatedText(info->lex_text);
							info->lex_text = NULL;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK) {
								return JSON_UNKNOWN_PROBLEM; // @todo return value according to the value returned from json_insert_child()
							}
							info->cursor = p_temp;
							p_temp = NULL;
							info->state = 5; // label, pre label:value separator
							break;
						case LEX_END_OBJECT:
							if(!info->cursor->P_Parent) {
								info->state = 99; // finished document. only accept whitespaces until EOF
							}
							else {
								info->cursor = info->cursor->P_Parent;
								switch(info->cursor->Type) {
								case json_t::tSTRING: // perform tree sanity checks
									assert(info->cursor->P_Parent);
									info->cursor = info->cursor->P_Parent;
									if(info->cursor->Type != json_t::tOBJECT) {
										return JSON_BAD_TREE_STRUCTURE;
									}
									else {
										info->state = 3;	/* finished adding a field to an object */
									}
									break;
								case json_t::tARRAY:
									info->state = 9;
									break;
								default:
									return JSON_BAD_TREE_STRUCTURE;
								}
							}
							break;
						case LEX_MORE:
							return JSON_INCOMPLETE_DOCUMENT;
							break;
						default:
							//fprintf(stderr, "JSON: state %d: defaulted\n", info->state);
							return JSON_MALFORMED_DOCUMENT;
							break;
					}
				}
				break;
			case 3: // finished adding a field to an object
				{
					// perform tree sanity checks
					assert(info->cursor);
					assert(info->cursor->Type == json_t::tOBJECT);
					switch(lexer(buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_VALUE_SEPARATOR:
							info->state = 4; /// sibling, post-object
							break;
						case LEX_END_OBJECT:
							if(!info->cursor->P_Parent)
								info->state = 99;	/* parse until EOF */
							else {
								info->cursor = info->cursor->P_Parent;
								switch(info->cursor->Type) {
									case json_t::tSTRING: // perform tree sanity checks
										assert(info->cursor->P_Parent);
										info->cursor = info->cursor->P_Parent;
										if(info->cursor->Type != json_t::tOBJECT)
											return JSON_BAD_TREE_STRUCTURE;
										else
											info->state = 3; // finished adding a field to an object
										break;
									case json_t::tARRAY:
										info->state = 9;
										break;
									default:
										return JSON_BAD_TREE_STRUCTURE;
								}
							}
							break;
						case LEX_MORE: return JSON_INCOMPLETE_DOCUMENT;
						default:
							//fprintf(stderr, "JSON: state %d: defaulted\n", info->state);
							return JSON_MALFORMED_DOCUMENT;
							break;
					}
				}
				break;
			case 4:	/* sibling, post-object */
				{
					assert(info->cursor);
					assert(info->cursor->Type == json_t::tOBJECT);
					switch(lexer(buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_STRING:
							if((p_temp = new json_t(json_t::tSTRING)) == NULL)
								return JSON_MEMORY;
							//p_temp->P_Text = rcs_unwrap(info->lex_text);
							p_temp->AssignAllocatedText(info->lex_text);
							info->lex_text = NULL;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK)
								return JSON_UNKNOWN_PROBLEM;
							info->cursor = p_temp;
							p_temp = 0;
							info->state = 5;
							break;
						case LEX_MORE: return JSON_INCOMPLETE_DOCUMENT;
						case LEX_INVALID_CHARACTER: return JSON_ILLEGAL_CHARACTER;
						default:
							//fprintf(stderr, "JSON: state %d: defaulted\n", info->state);
							return JSON_MALFORMED_DOCUMENT;
							break;
					}
				}
				break;
			case 5:	/* label, pre name separator */
				{
					/* perform tree sanity checks */
					assert(info->cursor);
					assert(info->cursor->Type == json_t::tSTRING);
					switch(lexer(buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_NAME_SEPARATOR: info->state = 6; break; /* label, pos label:value separator */
						case LEX_MORE: return JSON_INCOMPLETE_DOCUMENT; break;
						default:
							//fprintf(stderr, "JSON: state %d: defaulted\n", info->state);
							return JSON_MALFORMED_DOCUMENT;
							break;
					}
				}
				break;
			case 6:	/* label, pos name separator */
				{
					uint value;	/* to avoid redundant code */
					/* perform tree sanity checks */
					assert(info->cursor);
					assert(info->cursor->Type == json_t::tSTRING);
					switch(value = lexer(buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_STRING:
							if((p_temp = new json_t(json_t::tSTRING)) == NULL)
								return JSON_MEMORY;
							//p_temp->P_Text = rcs_unwrap(info->lex_text);
							p_temp->AssignAllocatedText(info->lex_text);
							info->lex_text = NULL;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK) {
								return JSON_UNKNOWN_PROBLEM; // @todo specify the exact error message
							}
							if(!info->cursor->P_Parent)
								info->state = 99; // finished document. only accepts whitespaces until EOF
							else
								info->cursor = info->cursor->P_Parent;
							p_temp = 0;
							info->state = 3; // finished adding a field to an object
							break;
						case LEX_NUMBER:
							if((p_temp = new json_t(json_t::tNUMBER)) == NULL)
								return JSON_MEMORY;
							//p_temp->P_Text = rcs_unwrap(info->lex_text);
							p_temp->AssignAllocatedText(info->lex_text);
							info->lex_text = NULL;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK) {
								return JSON_UNKNOWN_PROBLEM; // @todo specify the exact error message
							}
							if(!info->cursor->P_Parent)
								info->state = 99; // finished document. only accepts whitespaces until EOF
							else
								info->cursor = info->cursor->P_Parent;
							p_temp = 0;
							info->state = 3; // finished adding a field to an object
							break;
						case LEX_TRUE:
							if((p_temp = new json_t(json_t::tTRUE)) == NULL)
								return JSON_MEMORY;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK) {
								return JSON_UNKNOWN_PROBLEM; // @todo specify the exact error message
							}
							if(!info->cursor->P_Parent)
								info->state = 99; // finished document. only accepts whitespaces until EOF
							else
								info->cursor = info->cursor->P_Parent;
							p_temp = 0;
							info->state = 3; // finished adding a field to an object
							break;
						case LEX_FALSE:
							if((p_temp = new json_t(json_t::tFALSE)) == NULL)
								return JSON_MEMORY;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK)
								return JSON_UNKNOWN_PROBLEM; // @todo specify the exact error message
							if(!info->cursor->P_Parent)
								info->state = 99; // finished document. only accepts whitespaces until EOF
							else
								info->cursor = info->cursor->P_Parent;
							p_temp = 0;
							info->state = 3; // finished adding a field to an object
							break;
						case LEX_NULL:
							if((p_temp = new json_t(json_t::tNULL)) == NULL)
								return JSON_MEMORY;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK) {
								return JSON_UNKNOWN_PROBLEM; // @todo specify the exact error message 
							}
							if(!info->cursor->P_Parent)
								info->state = 99;	/* finished document. only accepts whitespaces until EOF */
							else
								info->cursor = info->cursor->P_Parent;
							p_temp = NULL;
							info->state = 3;	/* finished adding a field to an object */
							break;
						case LEX_BEGIN_OBJECT: info->state = 1; break;
						case LEX_BEGIN_ARRAY:  info->state = 7; break;
						case LEX_MORE: return JSON_INCOMPLETE_DOCUMENT;
						case LEX_MEMORY: return JSON_MEMORY;
						case LEX_INVALID_CHARACTER: return JSON_ILLEGAL_CHARACTER;
						default:
							//fprintf(stderr, "JSON: state %d: defaulted\n", info->state);
							return JSON_MALFORMED_DOCUMENT;
							break;
					}
				}
				break;
			case 7: // open array
				{
					if(info->cursor == NULL) {
						if((info->cursor = new json_t(json_t::tARRAY)) == NULL) {
							return JSON_MEMORY;
						}
					}
					else { // perform tree sanity checks
						assert(oneof2(info->cursor->Type, json_t::tARRAY, json_t::tSTRING));
						if((p_temp = new json_t(json_t::tARRAY)) == NULL) {
							return JSON_MEMORY;
						}
						if(json_insert_child(info->cursor, p_temp) != JSON_OK) {
							return JSON_UNKNOWN_PROBLEM;
						}
						info->cursor = p_temp;
						p_temp = 0;
					}
					info->state = 8;	/* just entered an array */
				}
				break;
			case 8: // just entered an array
				{
					// perform tree sanity checks
					assert(info->cursor);
					assert(info->cursor->Type == json_t::tARRAY);
					switch(lexer(buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_STRING:
							if((p_temp = new json_t(json_t::tSTRING)) == NULL)
								return JSON_MEMORY;
							//p_temp->P_Text = rcs_unwrap(info->lex_text);
							p_temp->AssignAllocatedText(info->lex_text);
							info->lex_text = NULL;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK)
								return JSON_UNKNOWN_PROBLEM;
							p_temp = 0;
							info->state = 9;	/* label, pre label:value separator */
							break;
						case LEX_NUMBER:
							if((p_temp = new json_t(json_t::tNUMBER)) == NULL)
								return JSON_MEMORY;
							//p_temp->P_Text = rcs_unwrap(info->lex_text);
							p_temp->AssignAllocatedText(info->lex_text);
							info->lex_text = NULL;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK)
								return JSON_UNKNOWN_PROBLEM;
							p_temp = NULL;
							info->state = 9;	/* label, pre label:value separator */
							break;
						case LEX_TRUE:
							if((p_temp = new json_t(json_t::tTRUE)) == NULL)
								return JSON_MEMORY;
							if(json_insert_child(info->cursor, p_temp) != JSON_OK)
								return JSON_UNKNOWN_PROBLEM;
							info->state = 9;	/* label, pre label:value separator */
							break;
						case LEX_FALSE:
							if((p_temp = new json_t(json_t::tFALSE)) == NULL)
								return JSON_MEMORY;
							else if(json_insert_child(info->cursor, p_temp) != JSON_OK)
								return JSON_UNKNOWN_PROBLEM;
							else
								info->state = 9;	/* label, pre label:value separator */
							break;
						case LEX_NULL:
							if((p_temp = new json_t(json_t::tNULL)) == NULL)
								return JSON_MEMORY;
							else if(json_insert_child(info->cursor, p_temp) != JSON_OK)
								return JSON_UNKNOWN_PROBLEM;
							else
								info->state = 9;	/* label, pre label:value separator */
							break;
						case LEX_BEGIN_ARRAY: info->state = 7; break; // open array
						case LEX_END_ARRAY:
							if(!info->cursor->P_Parent) {
								/*TODO implement this */
								info->state = 99;	/* finished document. only accept whitespaces until EOF */
							}
							else {
								info->cursor = info->cursor->P_Parent;
								switch(info->cursor->Type) {
								case json_t::tSTRING:
									if(!info->cursor->P_Parent)
										return JSON_BAD_TREE_STRUCTURE;
									else {
										info->cursor = info->cursor->P_Parent;
										if(info->cursor->Type != json_t::tOBJECT) {
											return JSON_BAD_TREE_STRUCTURE;
										}
										info->state = 3;	/* followup to adding child to array */
									}
									break;
								case json_t::tARRAY: info->state = 9; break; // followup to adding child to array
								default: return JSON_BAD_TREE_STRUCTURE;
								}
							}
							break;
						case LEX_BEGIN_OBJECT: info->state = 1; break; // open object
						case LEX_MORE: return JSON_INCOMPLETE_DOCUMENT;
						case LEX_INVALID_CHARACTER: return JSON_ILLEGAL_CHARACTER;
						default:
							// fprintf(stderr, "JSON: state %d: defaulted\n", info->state);
							return JSON_MALFORMED_DOCUMENT;
							break;
					}
				}
				break;
			case 9: // followup to adding child to array
				{
					/*TODO perform tree sanity checks */
					assert(info->cursor);
					switch(lexer (buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_VALUE_SEPARATOR:
							info->state = 8;
							break;
						case LEX_END_ARRAY:
							if(!info->cursor->P_Parent)
								info->state = 99; // finished document. only accept whitespaces until EOF
							else {
								info->cursor = info->cursor->P_Parent;
								switch(info->cursor->Type) {
									case json_t::tSTRING:
										if(!info->cursor->P_Parent) {
											info->state = 99; // finished document. only accept whitespaces until EOF
										}
										else {
											info->cursor = info->cursor->P_Parent;
											if(info->cursor->Type != json_t::tOBJECT)
												return JSON_BAD_TREE_STRUCTURE;
											else
												info->state = 3; // followup to adding child to array
										}
										break;
									case json_t::tARRAY: info->state = 9; break; // followup to adding child to array
									default:
										return JSON_BAD_TREE_STRUCTURE;
								}
							}
							break;
						case LEX_MORE:
							return JSON_INCOMPLETE_DOCUMENT;
							break;
						default:
							// fprintf(stderr, "JSON: state %d: defaulted\n", info->state);
							return JSON_MALFORMED_DOCUMENT;
							break;
					}
				}
				break;
			case 99: // finished document. only accept whitespaces until EOF
				{
					// perform tree sanity check
					assert(info->cursor->P_Parent == NULL);
					switch(lexer(buffer, &info->p, &info->lex_state, &info->lex_text)) {
						case LEX_MORE: return JSON_WAITING_FOR_EOF; break;
						case LEX_MEMORY: return JSON_MEMORY; break;
						default: return JSON_MALFORMED_DOCUMENT; break;
					}
				}
				break;
			default:
				//fprintf(stderr, "JSON: invalid parser state %d: defaulted\n", info->state);
				return JSON_UNKNOWN_PROBLEM;
		}
	}
	info->p = NULL;
	if(info->state == 99)
		return JSON_WAITING_FOR_EOF;
	else
		return JSON_INCOMPLETE_DOCUMENT;
}

enum json_error json_parse_document(json_t ** root, const char *text)
{
	enum json_error error = JSON_OK;
	assert(root);
	assert(*root == NULL);
	assert(text);
	// initialize the parsing structure
	json_parsing_info * jpi = (json_parsing_info *)SAlloc::M(sizeof(json_parsing_info));
	if(jpi == NULL) {
		error = JSON_MEMORY;
	}
	else {
		json_jpi_init(jpi);
		error = json_parse_fragment(jpi, text);
		if(oneof2(error, JSON_WAITING_FOR_EOF, JSON_OK)) {
			*root = jpi->cursor;
			SAlloc::F(jpi);
			error = JSON_OK;
		}
		else
			SAlloc::F(jpi);
	}
	return error;
}

enum json_error json_saxy_parse(json_saxy_parser_status *jsps, json_saxy_functions * jsf, char c)
{
	// @todo handle a string instead of a single char
	//RcString * temp = 0;
	// make sure everything is in it's place
	assert(jsps);
	assert(jsf);
	// goto where we left off
	switch(jsps->state) {
		case 0:  goto state0;  break; // general state. everything goes.
		case 1:  goto state1;  break; // parse string
		case 2:  goto state2;  break; // parse string: escaped character
		case 3:  goto state3;  break; // parse string: escaped unicode 1
		case 4:  goto state4;  break; // parse string: escaped unicode 2
		case 5:  goto state5;  break; /* parse string: escaped unicode 3 */
		case 6:  goto state6;  break; /* parse string: escaped unicode 4 */
		case 7:  goto state7;  break; /* parse true: tr */
		case 8:  goto state8;  break; /* parse true: tru */
		case 9:  goto state9;  break; /* parse true: true */
		case 10: goto state10; break; /* parse false: fa */
		case 11: goto state11; break; /* parse false: fal */
		case 12: goto state12; break; /* parse false: fals */
		case 13: goto state13; break; /* parse false: false */
		case 14: goto state14; break; /* parse null: nu */
		case 15: goto state15; break; /* parse null: nul */
		case 16: goto state16; break; /* parse null: null */
		case 17: goto state17; break; /* parse number: 0 */
		case 18: goto state18; break; /* parse number: start fraccional part */
		case 19: goto state19; break; /* parse number: fraccional part */
		case 20: goto state20; break; /* parse number: start exponent part */
		case 21: goto state21; break; /* parse number: exponent part */
		case 22: goto state22; break; /* parse number: exponent sign part */
		case 23: goto state23; break; /* parse number: start negative */
		case 24: goto state24; break; /* parse number: decimal part */
		case 25: goto state25; break; /* open object */
		case 26: goto state26; break; /* close object/array */
		case 27: goto state27; break; /* sibling followup */
		default: return JSON_UNKNOWN_PROBLEM; /* oops... this should never be reached */
	}
	state0: // starting point
	{
		switch(c) {
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D':	/* JSON insignificant white spaces */
				break;
			case '\"':	/* starting a string */
				jsps->string_length_limit_reached = 0;
				jsps->state = 1;
				break;
			case '{':
				if(jsf->open_object)
					jsf->open_object();
				jsps->state = 25;	/*open object */
				break;
			case '}':
				if(jsf->close_object)
					jsf->close_object();
				jsps->state = 26;	/* close object/array */
				break;
			case '[':
				if(jsf->open_array)
					jsf->open_array();
				// jsps->state = 0; // redundant
				break;
			case ']':
				if(jsf->close_array)
					jsf->close_array();
				jsps->state = 26;	/* close object/array */
				break;
			case 't': jsps->state = 7; break; // parse true: tr
			case 'f': jsps->state = 10; break; // parse false: fa
			case 'n': jsps->state = 14; break; // parse null: nu
			case ':':
				if(jsf->label_value_separator)
					jsf->label_value_separator();
				// jsps->state = 0; // redundant
				break;
			case ',':
				if(jsf->sibling_separator)
					jsf->sibling_separator();
				jsps->state = 27;	/* sibling followup */
				break;
			case '0':
				jsps->string_length_limit_reached = 0;
				jsps->state = 17;	/* parse number: 0 */
				if((jsps->temp = rcs_create(5)) == NULL)
					return JSON_MEMORY;
				rcs_catc((jsps->temp), '0');
				break;
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				jsps->string_length_limit_reached = 0;
				jsps->state = 24;	/* parse number: decimal */
				if((jsps->temp = rcs_create(5)) == NULL)
					return JSON_MEMORY;
				rcs_catc((jsps->temp), c);
				break;
			case '-':
				jsps->string_length_limit_reached = 0;
				jsps->state = 23;	/* number: */
				jsps->temp = NULL;
				if((jsps->temp = rcs_create(5)) == NULL)
					return JSON_MEMORY;
				rcs_catc((jsps->temp), '-');
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
state1: // parse string
	{
		switch(c) {
			case '\\':
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < (JSON_MAX_STRING_LENGTH-1)) // check if there is space for a two character escape sequence
						rcs_catc((jsps->temp), '\\');
					else
						jsps->string_length_limit_reached = 1;
				}
				jsps->state = 2;	/* parse string: escaped character */
				break;
			case '\"':	/* end of string */
				if(jsps->temp) {
					jsps->state = 0;	/* starting point */
					if(jsf->new_string)
						jsf->new_string(jsps->temp->P_Text); /*copied or integral? */
					rcs_free(&jsps->temp);
				}
				else
					return JSON_UNKNOWN_PROBLEM;
				break;
			default:
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < JSON_MAX_STRING_LENGTH) // check if there is space for a two character escape sequence
						rcs_catc((jsps->temp), c);
					else
						jsps->string_length_limit_reached = 1;
				}
				break;
		}
		return JSON_OK;
	}
state2: // parse string: escaped character
		switch(c) {
			case '\"':
			case '\\':
			case '/':
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < JSON_MAX_STRING_LENGTH)
						rcs_catc((jsps->temp), c);
					else
						jsps->string_length_limit_reached = 1;
				}
				break;
			case 'u':
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < (JSON_MAX_STRING_LENGTH - 4))
						rcs_catc((jsps->temp), 'u');
					else
						jsps->string_length_limit_reached = 1;
				}
				jsps->state = 3;	/* parse string: escaped unicode 1; */
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
state3: // parse string: escaped unicode 1
		if(ishex(c)) {
			if(!jsps->string_length_limit_reached) {
				if(jsps->temp->Len() < (JSON_MAX_STRING_LENGTH - 3))
					rcs_catc((jsps->temp), 'u');
				else
					jsps->string_length_limit_reached = 1;
			}
			jsps->state = 4; // parse string. escaped unicode 2
			return JSON_OK;
		}
		else
			return JSON_ILLEGAL_CHARACTER;
state4: // parse string: escaped unicode 2
		if(ishex(c)) {
			if(!jsps->string_length_limit_reached) {
				if(jsps->temp->Len() < JSON_MAX_STRING_LENGTH - 2)
					rcs_catc((jsps->temp), c);
				else
					jsps->string_length_limit_reached = 1;
			}
			jsps->state = 5;	/* parse string. escaped unicode 3 */
			return JSON_OK;
		}
		else
			return JSON_ILLEGAL_CHARACTER;
state5: // parse string: escaped unicode 3
		if(ishex(c)) {
			if(!jsps->string_length_limit_reached) {
				if(jsps->temp->Len() < (JSON_MAX_STRING_LENGTH-1))
					rcs_catc((jsps->temp), c);
				else
					jsps->string_length_limit_reached = 1;
			}
			jsps->state = 6; // parse string. escaped unicode 4
			return JSON_OK;
		}
		else
			return JSON_ILLEGAL_CHARACTER;
state6: // parse string: escaped unicode 4
		if(ishex(c)) {
			if(!jsps->string_length_limit_reached) {
				if(jsps->temp->Len() < JSON_MAX_STRING_LENGTH)
					rcs_catc((jsps->temp), c);
				else
					jsps->string_length_limit_reached = 1;
			}
			jsps->state = 1;	/* parse string */
			return JSON_OK;
		}
		else
			return JSON_ILLEGAL_CHARACTER;
state7: // parse true: tr
		if(c != 'r') {
			return JSON_ILLEGAL_CHARACTER;
		}
		else {
			jsps->state = 8; // parse true: tru
			return JSON_OK;
		}
state8: // parse true: tru
		if(c != 'u')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 9;	/* parse true: true */
			return JSON_OK;
		}
state9: // parse true: true
		if(c != 'e')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 0;	/* back to general state. */
			if(jsf->new_true)
				jsf->new_true();
			return JSON_OK;
		}
state10: // parse false: fa
		if(c != 'a')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 11;	/* parse true: fal */
			return JSON_OK;
		}
state11: // parse false: fal
		if(c != 'l')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 12;	/* parse true: fals */
			return JSON_OK;
		}
state12: // parse false: fals
		if(c != 's')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 13;	/* parse true: false */
			return JSON_OK;
		}
state13: // parse false: false
		if(c != 'e')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 0;	/* general state. everything goes. */
			if(jsf->new_false)
				jsf->new_false();
			return JSON_OK;
		}
state14: // parse null: nu
		if(c != 'u')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 15;	/* parse null: nul */
			return JSON_OK;
		}
state15: // parse null: nul
		if(c != 'l')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 16;	/* parse null: null */
			return JSON_OK;
		}
state16: // parse null: null
		if(c != 'l')
			return JSON_ILLEGAL_CHARACTER;
		else {
			jsps->state = 0;	/* general state. everything goes. */
			if(jsf->new_null)
				jsf->new_null();
			return JSON_OK;
		}
state17: // parse number: 0
	{
		switch(c) {
		case '.':
			if((jsps->temp = rcs_create(5)) == NULL) {
				return JSON_MEMORY;
			}
			rcs_catc((jsps->temp), '.');
			jsps->state = 18;	/* parse number: fraccional part */
			break;
		case '\x20':
		case '\x09':
		case '\x0A':
		case '\x0D':	/* JSON insignificant white spaces */
			if(!jsps->temp)
				return JSON_MEMORY;
			if(jsf->new_number)
				jsf->new_number((jsps->temp)->P_Text);
			rcs_free(&jsps->temp);
			jsps->state = 0;
			break;
		case '}':
			if(!jsps->temp)
				return JSON_MEMORY;
			if(jsf->new_number)
				jsf->new_number((jsps->temp)->P_Text);
			rcs_free(&jsps->temp);
			if(jsf->open_object)
				jsf->close_object();
			jsps->state = 26;	/* close object/array */
			break;
		case ']':
			if(!jsps->temp)
				return JSON_MEMORY;
			if(jsf->new_number)
				jsf->new_number((jsps->temp)->P_Text);
			rcs_free(&jsps->temp);
			if(jsf->open_object)
				jsf->close_array();
			jsps->state = 26; // close object/array
			break;
		case ',':
			if(!jsps->temp)
				return JSON_MEMORY;
			if(jsf->new_number)
				jsf->new_number((jsps->temp)->P_Text);
			rcs_free(&jsps->temp);
			if(jsf->open_object)
				jsf->label_value_separator();
			jsps->state = 27;	/* sibling followup */
			break;
		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}
state18: // parse number: start fraccional part
      	if(isdec(c)) {
			if(!jsps->string_length_limit_reached) {
				if(jsps->temp->Len() < (JSON_MAX_STRING_LENGTH / 2))
					rcs_catc((jsps->temp), c);
				else
					jsps->string_length_limit_reached = 1;
			}
			jsps->state = 19;	/* parse number: fractional part */
			return JSON_OK;
      	}
      	else
			return JSON_ILLEGAL_CHARACTER;
state19: // parse number: fraccional part
	{
		switch(c) {
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < (JSON_MAX_STRING_LENGTH/2))
						rcs_catc((jsps->temp), c);
					else
						jsps->string_length_limit_reached = 1;
				}
				// jsps->state = 19; // parse number: fractional part
				break;
			case 'e':
			case 'E':
				rcs_catc((jsps->temp), c);
				jsps->state = 20;	/* parse number: start exponent part */
				break;
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D':	/* JSON insignificant white spaces */
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				jsps->state = 0;
				break;
			case '}':
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				if(jsf->open_object)
					jsf->close_object();
				jsps->state = 26;	/* close object/array */
				break;
			case ']':
				if(jsf->new_number) {
					if(!jsps->temp)
						return JSON_MEMORY;
					jsf->new_number((jsps->temp)->P_Text);
					rcs_free(&jsps->temp);
				}
				else {
					rcs_free(&jsps->temp);
					jsps->temp = NULL;
				}
				if(jsf->open_object)
					jsf->close_array();
				jsps->state = 26;	/* close object/array */
				break;
			case ',':
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				if(jsf->label_value_separator)
					jsf->label_value_separator();
				jsps->state = 27;	/* sibling followup */
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
	state20:			/* parse number: start exponent part */
	{
		switch(c) {
			case '+':
			case '-':
				jsps->string_length_limit_reached = 0;
				rcs_catc((jsps->temp), c);
				jsps->state = 22;	/* parse number: exponent sign part */
				break;
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < JSON_MAX_STRING_LENGTH)
						rcs_catc((jsps->temp), c);
					else
						jsps->string_length_limit_reached = 1;
				}
				jsps->state = 21;	/* parse number: exponent part */
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
state21: // parse number: exponent part
	{
		switch(c) {
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < JSON_MAX_STRING_LENGTH)
						rcs_catc((jsps->temp), c);
					else
						jsps->string_length_limit_reached = 1;
				}
				// jsps->state = 21; // parse number: exponent part
				break;
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D':	/* JSON insignificant white spaces */
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				jsps->state = 0;
				break;
			case '}':
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				if(jsf->open_object)
					jsf->close_object();
				jsps->state = 26;	/* close object */
				break;
			case ']':
				if(jsf->new_number) {
					if(!jsps->temp)
						return JSON_MEMORY;
					jsf->new_number((jsps->temp)->P_Text);
					ZFREE(jsps->temp);
				}
				else {
					ZFREE(jsps->temp);
				}
				if(jsf->open_object)
					jsf->close_array();
				jsps->state = 26;	/* close object/array */
				break;
			case ',':
				if(jsf->new_number) {
					if(!jsps->temp)
						return JSON_MEMORY;
					jsf->new_number((jsps->temp)->P_Text);
					ZFREE(jsps->temp);
				}
				else {
					ZFREE(jsps->temp);
				}
				if(jsf->label_value_separator)
					jsf->label_value_separator();
				jsps->state = 27;	/* sibling followup */
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
state22: // parse number: start exponent part
		if(isdec(c)) {
			if(!jsps->string_length_limit_reached) {
				if(jsps->temp->Len() < JSON_MAX_STRING_LENGTH)
					rcs_catc((jsps->temp), c);
				else
					jsps->string_length_limit_reached = 1;
			}
			jsps->state = 21;	/* parse number: exponent part */
			return JSON_OK;
		}
		else
			return JSON_ILLEGAL_CHARACTER;
state23: // parse number: start negative
	{
		switch(c) {
			case '0':
				rcs_catc((jsps->temp), c);
				jsps->state = 17;	/* parse number: 0 */
				break;
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < (JSON_MAX_STRING_LENGTH / 2)) {
						if((jsps->temp = rcs_create(5)) == NULL) {
							return JSON_MEMORY;
						}
						rcs_catc((jsps->temp), c);
						jsps->string_length_limit_reached = 1;
					}
				}
				jsps->state = 24;	/* parse number: start decimal part */
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
state24: // parse number: decimal part
	{
		switch(c) {
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				if(!jsps->string_length_limit_reached) {
					if(jsps->temp->Len() < (JSON_MAX_STRING_LENGTH / 2)) {
						if((jsps->temp = rcs_create(5)) == NULL) {
							return JSON_MEMORY;
						}
						rcs_catc((jsps->temp), c);
					}
					else
						jsps->string_length_limit_reached = 1;
				}
				/* jsps->state = 24; // parse number: decimal part*/
				break;
			case '.':
				if((jsps->temp = rcs_create(5)) == NULL) {
					return JSON_MEMORY;
				}
				rcs_catc((jsps->temp), '.');
				jsps->state = 18;	/* parse number: start exponent part */
				break;
			case 'e':
			case 'E':
				if((jsps->temp = rcs_create(5)) == NULL) {
					return JSON_MEMORY;
				}
				rcs_catc((jsps->temp), c);
				jsps->string_length_limit_reached = 0;	/* reset to accept the exponential part */
				jsps->state = 20;	/* parse number: start exponent part */
				break;
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D':	/* JSON insignificant white spaces */
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				jsps->state = 0;
				break;
			case '}':
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				if(jsf->open_object)
					jsf->close_object();
				jsps->state = 26;	/* close object/array */
				break;
			case ']':
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				if(jsf->open_object)
					jsf->close_array();
				jsps->state = 26;	/* close object/array */
				break;
			case ',':
				if(!jsps->temp)
					return JSON_MEMORY;
				if(jsf->new_number)
					jsf->new_number((jsps->temp)->P_Text);
				rcs_free(&jsps->temp);
				if(jsf->label_value_separator)
					jsf->label_value_separator();
				jsps->state = 27;	/* sibling followup */
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
state25:			/* open object */
	{
		switch(c) {
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D':	/* JSON insignificant white spaces */
				break;
			case '\"':
				jsps->temp = NULL;
				jsps->state = 1;
				break;
			case '}':
				if(jsf->close_object)
					jsf->close_object();
				jsps->state = 26;	/* close object */
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
state26: // close object/array
	{
		switch(c) {
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D': // JSON insignificant white spaces
				break;
			case '}':
				if(jsf->close_object)
					jsf->close_object();
				// jsp->state = 26; // close object
				break;
			case ']':
				if(jsf->close_array)
					jsf->close_array();
				// jsps->state = 26;       // close object/array
				break;
			case ',':
				if(jsf->sibling_separator)
					jsf->sibling_separator();
				jsps->state = 27;	/* sibling followup */
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
state27: // sibling followup
	{
		switch(c) {
			case '\x20':
			case '\x09':
			case '\x0A':
			case '\x0D': // JSON insignificant white spaces
				break;
			case '\"':
				jsps->state = 1;
				jsps->temp = NULL;
				break;
			case '{':
				if(jsf->open_object)
					jsf->open_object();
				jsps->state = 25;	/*open object */
				break;
			case '[':
				if(jsf->open_array)
					jsf->open_array();
				// jsps->state = 0; // redundant
				break;
			case 't': jsps->state = 7; break; // parse true: tr
			case 'f': jsps->state = 10; break; // parse false: fa
			case 'n': jsps->state = 14; break; // parse null: nu
			case '0':
				jsps->state = 17; // parse number: 0
				if((jsps->temp = rcs_create(5)) == NULL) {
					return JSON_MEMORY;
				}
				rcs_catc((jsps->temp), '0');
				break;
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				jsps->state = 24;	/* parse number: decimal */
				if((jsps->temp = rcs_create(5)) == NULL) {
					return JSON_MEMORY;
				}
				rcs_catc((jsps->temp), c);
				break;
			case '-':
				jsps->state = 23;	/* number: */
				if((jsps->temp = rcs_create(RSTRING_DEFAULT)) == NULL) {
					return JSON_MEMORY;
				}
				rcs_catc((jsps->temp), '-');
				break;
			default:
				return JSON_ILLEGAL_CHARACTER;
				break;
		}
		return JSON_OK;
	}
	return JSON_UNKNOWN_PROBLEM;
}

json_t * json_find_first_label(const json_t * object, const char * text_label)
{
	json_t * cursor;
	assert(object);
	assert(text_label);
	assert(object->Type == json_t::tOBJECT);
	for(cursor = object->P_Child; cursor; cursor = cursor->P_Next)
		if(cursor->Text == text_label)
			break;
	return cursor;
}

const char * json_get_value(const json_t * object, const char * text_label)
{
	json_t * cursor = json_find_first_label(object, text_label);
	return (cursor && cursor->P_Child) ? cursor->P_Child->Text.cptr() : 0;
}

json_t * json_process(json_t * cursor)
{
	int exit_flg = 0;
	switch(cursor->Type) {
		case json_t::tARRAY:
		case json_t::tOBJECT:
			if(cursor->P_Child) {
				cursor = cursor->P_Child;
				return cursor;
			}
			else
				cursor = cursor->P_Next;
			break;
	}
	if(cursor->P_Child) {
		cursor = cursor->P_Child;
	}
	else {
		while(1) {
			if(cursor->P_Parent == NULL)
				break;
			else if(cursor->P_Next) {
				cursor = cursor->P_Next;
				break;
			}
			else {
				cursor = cursor->P_Parent;
				continue;
			}
		}
	}
	return cursor;
}
//
//
//
#if SLTEST_RUNNING // {
//
// @construction
//
static SString & Helper_JsonToXmlEncTag(const char * pText, const char * pDefText, SString & rBuf)
{
	if(pText) {
		XMLReplaceSpecSymb(rBuf = pText, "&<>\'");
	}
	else {
		rBuf = pDefText;
	}
	rBuf.ReplaceChar(' ', '_');
	return rBuf;
}

static SString & Helper_JsonToXmlEncText(const char * pText, const char * pDefText, SString & rBuf)
{
	if(pText) {
		XMLReplaceSpecSymb(rBuf = pText, "&<>\'");
	}
	else {
		rBuf = pDefText;
	}
	return rBuf;
}

static int Helper_JsonToXmlDOM(json_t * pNode, int superJsonType, SXml::WDoc & rXmlDoc)
{
	int    ok = 1;
	SString temp_buf;
	SString val_buf;
	for(json_t * p_cur = pNode; p_cur; p_cur = p_cur->P_Next) {
		switch(p_cur->Type) {
			case json_t::tARRAY:
				{
					//SXml::WNode xn(rXmlDoc, Helper_JsonToXmlEncTag((pNode->P_Parent ? pNode->P_Parent->P_Text : 0), "array", temp_buf));
					THROW(Helper_JsonToXmlDOM(p_cur->P_Child, p_cur->Type, rXmlDoc));   // @recursion
				}
				break;
			case json_t::tOBJECT:
				{
					//SXml::WNode xn(rXmlDoc, Helper_JsonToXmlEncTag((pNode->P_Parent ? pNode->P_Parent->P_Text : 0), "object", temp_buf));
					THROW(Helper_JsonToXmlDOM(p_cur->P_Child, p_cur->Type, rXmlDoc));   // @recursion
				}
				break;
			case json_t::tSTRING:
			case json_t::tNUMBER:
				{
					SXml::WNode xn(rXmlDoc, Helper_JsonToXmlEncTag(p_cur->Text, "value", temp_buf));
					if(p_cur->P_Child) {
						THROW(Helper_JsonToXmlDOM(p_cur->P_Child, p_cur->Type, rXmlDoc));   // @recursion
					}
					else {
						//SXml::WNode xn(rXmlDoc, "value", Helper_JsonToXmlEncText(p_cur->P_Text, 0, val_buf));
					}
				}
				break;
			case json_t::tTRUE:
				if(p_cur->P_Child) {
					THROW(Helper_JsonToXmlDOM(p_cur->P_Child, p_cur->Type, rXmlDoc));   // @recursion
				}
				else {
					SXml::WNode xn(rXmlDoc, Helper_JsonToXmlEncTag(p_cur->Text, "bool", temp_buf), "true");
				}
				break;
			case json_t::tFALSE:
				if(p_cur->P_Child) {
					THROW(Helper_JsonToXmlDOM(p_cur->P_Child, p_cur->Type, rXmlDoc));   // @recursion
				}
				else {
					SXml::WNode xn(rXmlDoc, Helper_JsonToXmlEncTag(p_cur->Text, "bool", temp_buf), "false");
				}
				break;
			case json_t::tNULL:
				if(p_cur->P_Child) {
					THROW(Helper_JsonToXmlDOM(p_cur->P_Child, p_cur->Type, rXmlDoc));   // @recursion
				}
				else {
					SXml::WNode xn(rXmlDoc, Helper_JsonToXmlEncTag(p_cur->Text, "null", temp_buf), "null");
				}
				break;
			default:
				break;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI JsonToXmlDOM(const char * pJsonText, SString & rXmlText)
{
	rXmlText = 0;
	int    ok = 1;
	json_t * p_json_doc = NULL;
	xmlTextWriter * p_writer = 0;
	xmlBuffer * p_xml_buf = 0;
	THROW(json_parse_document(&p_json_doc, pJsonText) == JSON_OK);
	{
		THROW(p_xml_buf = xmlBufferCreate());
		THROW(p_writer = xmlNewTextWriterMemory(p_xml_buf, 0));
		xmlTextWriterSetIndent(p_writer, 1);
        {
        	SXml::WDoc _doc(p_writer, cpUTF8);
        	THROW(Helper_JsonToXmlDOM(p_json_doc, 0, _doc));
			xmlTextWriterFlush(p_writer);
			rXmlText.CopyFromN((char *)p_xml_buf->content, p_xml_buf->use);
        }
	}
	CATCHZOK
	xmlFreeTextWriter(p_writer);
	xmlBufferFree(p_xml_buf);
	json_free_value(&p_json_doc);
	return ok;
}

int SLAPI XmlToJsonDOM(const char * pXmlText, SString & rJsonText)
{
	int    ok = 1;
	return ok;
}

SLTEST_R(JSON)
{
	int    ok = 1;
	struct InputEntry {
		const char * P_InFileName;
		int    Valid;
	};
	static const InputEntry inp_entries[] = {
		{ "binary_dependencies.json", 1 },
		{ "tests.json", 1 },
		{ "pass1.json", 1 },
		{ "pass2.json", 1 },
		{ "pass3.json", 1 },
		{ "USStates.json", 1 },
	};
	for(uint i = 0; i < SIZEOFARRAY(inp_entries); i++) {
		SString in_file_name = MakeInputFilePath(inp_entries[i].P_InFileName);
		THROW(fileExists(in_file_name));
		{
			SString json_buf;
			SString xml_buf;
			SString temp_buf;
			SFile f_in(in_file_name, SFile::mRead);
			THROW(f_in.IsValid());
			{
				STempBuffer rd_buf(4096);
				size_t actual_size = 0;
				THROW(rd_buf.IsValid());
				while(f_in.Read(rd_buf, rd_buf.GetSize(), &actual_size) && actual_size) {
					json_buf.CatN(rd_buf, actual_size);
				}
				SLTEST_CHECK_NZ(JsonToXmlDOM(json_buf, xml_buf));
				{
					SPathStruc ps(in_file_name);
					ps.Nam.CatChar('-').Cat("result");
					ps.Ext = "xml";
					ps.Drv.Z();
					ps.Dir.Z();
					ps.Merge(temp_buf);
					SString out_file_name = MakeOutputFilePath(temp_buf);
					SFile f_out(out_file_name, SFile::mWrite);
					THROW(f_out.IsValid());
					f_out.WriteLine(xml_buf);
				}
			}
		}
	}
	CATCHZOK
	return CurrentStatus;
}

#endif