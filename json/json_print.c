#include "json_print.h"
#include <stdint.h>
#include <stdio.h>

/* ************************************************************************** */

/*  This is a pointer to the printer function that was passed as the first
    argument to json_print(). This could have been passed down the call tree as
    an argument, and some people probably would have preferred that, but since
    evaluate_node_list() can end up being called recursively AND the value of
    out can't change in the middle of that, I think it makes more sense to pull
    this out to file scope to help limit stack growth.

    typedef void (*printer_t)(const char *);

*/
static printer_t out;

/* ************************************************************************** */

/*  JSON strings must be surrounded by double-quotes, and since double-quotes
    inside string literals have to be escaped, we'll provide a function that
    does it for you.
*/
static void print_json_string(char *string) {
    out("\"");
    out(string);
    out("\"");
}

/* -------------------------------------------------------------------------- */

static void evaluate_node_list(const json_node_t *nodeList); // forward dec

/*  Any non-control node is evaluated here.

    A node has two elements: a type and a void pointer to its contents.

    Evaluating a node involves looking at the type of that node, performing the
    relevant typecast on the contents pointer, converting those contents into a
    string, and then printing it.

    The conversion to string is done using sprintf instead of printf(), because
    printf() has a fixed destination of whatever putc() goes. Since the output
    of json_print() is retargetable, we format with sprintf(), but the result in
    a local char buffer, and then print that char buffer using the function
    pointer that was given to json_print() and copied into 'out'.
*/
static void evaluate_node(const json_node_t *node) {
    char buffer[50] = {0};

    switch (node->type) {
        case nNodeList:
            evaluate_node_list((const json_node_t *)node->contents);
            return;
        case nKey:
            print_json_string((char *)node->contents);
            out(":");
            return;
        case nString:
            print_json_string((char *)node->contents);
            return;
        case nFloat:
            sprintf(&buffer[0], "%f", *(double *)node->contents);
            out(buffer);
            return;
        case nU16:
            sprintf(&buffer[0], "%u", *(uint16_t *)node->contents);
            out(buffer);
            return;
        case nU32:
            sprintf(&buffer[0], "%lu", *(uint32_t *)node->contents);
            out(buffer);
            return;
        case nNull:
            out("null");
            return;
        default: // type not supported
            out("null");
            return;
    }
}

/* -------------------------------------------------------------------------- */

/*  braceDepth:

    JSON objects are delimited by curly braces: '{' and '}'. It's REQUIRED for
    these braces to match. To help make this easier, evaluate_node_list() keeps
    track of every curly brace it prints. This has two major behavioral
    consequences:

    1)  Printing a '}' that matches against the very first opening brace means
        that this JSON object MUST be over. Therefore, if that happens, we'll
        print the brace and return. If there were still nodes in the list, then
        the JSON object was defined improperly.

    2)  Encountering a control node containing "\e" means that we've hit the end
        of the node list we're evaluating and should return. Assuming a simple
        node list(a list without any nNodeList nodes), hitting "\e" implies that
        we're completely done and should return. However, if there are still
        unmatched open braces, we haven't produced valid JSON output. Therefore,
        before returning, we'll print close braces until we've balanced all the
        open braces.
*/
static uint8_t braceDepth = 0;

/*  recursionCount:

    In the description of braceDepth(see above), we learned about how
    evaluate_node_list() adds missing close braces when it hits the end of a
    node list(eg, a control node containing "\e"). There's a big problem with
    this: a node list can have a reference to another node list!

    This is an important feature for writing the definitions of JSON objects, so
    you can factor out common sections of related JSON objects, or just to break
    a big definition into smaller, easier to read pieces.

    This presents a problem when evaluating a list, because now you need to know
    whether you're looking at the 'parent' list or a 'child' list before you
    can decide what you to do when you hit "\e".

    One solution I tried was adding another control node symbol, so parent lists
    would end with "\e" and child lists would end with "\b". This was a bad
    solution because it made it harder to write object definitions, and it meant
    you had to decide up front whether a list could be included in another list.

    Instead, if we increment recursionCount at the beginning of every node list,
    whether it's the parent or the child, and we decrement it every time we
    evaluate a "\e" node, we'll know the difference between the end of a parent
    list and the end of a child list.

    Now, we can control the previously described brace matching feature and only
    do it when we're at the end of the starting(parent) node list.
*/
static uint8_t recursionCount = 0;

/*  evaluate_node_list() builds a JSON string by iterating through an array of
    json_node_t's.

    This function handles control nodes and keeping track of the structure of
    the JSON object we're printing, and delegates most of the formatting and
    printing to evaluate_node().

    ! evaluate_node_list() can be called recursively !
    One of the features of the nodal JSON definition is the ability to reference
    other lists of nodes. There's a special node type that indicates this and
    contains a pointer to the list to be included. When evaluate_node() hits one
    of these special nodes, it will call evaluate_node_list() and pass it a
    pointer to the included list.

    ! This function provides NO PROTECTION against circular inclusion. !
    If listA includes listB, and listB includes listA, these fucntions will
    follow the trail until the system overflows its stack and (hopefully)
    triggers a RESET. It's your responsibility to make sure your JSON node lists
    aren't dangerous.
*/
static void evaluate_node_list(const json_node_t *list) {
    const json_node_t *currentNode;
    const json_node_t *nextNode;

    recursionCount++;

    while (1) {
        currentNode = list;
        nextNode = ++list;

        if (currentNode->type == nControl) {
            char controlChar = ((const char *)currentNode->contents)[0];
            switch (controlChar) {
                case '{':
                    braceDepth++;
                    out("{");
                    break;
                case '}':
                    braceDepth--;
                    out("}");
                    if (braceDepth == 0) {
                        return;
                    }
                    if (nextNode->type != nControl) {
                        out(",");
                    }
                    break;
                case '\e':
                    recursionCount--;
                    if (recursionCount == 0) {
                        while (braceDepth--) {
                            out("}");
                        }
                    }
                    return;
            }
        } else {
            evaluate_node(currentNode);

            // printing a comma is a little complicated, see [0] for more info
            if (nextNode->type == nKey) {
                out(",");
            } else if (nextNode->type == nNodeList) {
                const json_node_t *lookAhead = nextNode->contents;
                if (lookAhead->type == nKey) {
                    out(",");
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

/*  A JSON object is printed by stepping through its definition one node at a
    time until we're done. Each node is evaluated one by one and its contents
    converted to a string and passed to the printing function defined as
    'destination'.





*/
void json_print(printer_t destination, const json_node_t *nodeList) {
    // copy the destination pointer to a static variable, so we don't have to
    // keep passing it down the stack
    out = destination;

    // Reset the internal state used to keep track of the JSON structure
    braceDepth = 0;
    recursionCount = 0;

    //
    evaluate_node_list(nodeList);
}

/* ************************************************************************** */

/*  [0] Additional information on JSON commas

    JSON requires a comma between two key:value pairs, like this:
            necessary and correct
                   v
        { key:value, key:value }

    So we could follow every value with a comma, but then we'd get this:
                    invalid and illegal(why can't JSON just ignore this?)
                              v
        { key:value, key:value, }

    So instead let's look ahead and print a comma if the NEXT node is a key:
        invalid and illegal(this one is more reasonable)
          v
        { , key:value, key:value }

    This is actually easier to fix than the first failure. Let's write the rule
    in pseudo-C (you'll need to know about node types to understand this):
        if(currentNode.type != nControl && nextNode.type == nKey){
            print_comma();
        }

    Example of an invalid place to print a comma:
        currentNode IS a control node
        v
        { key:value, key:value }
           ^
         nextNode IS a key node

    Example of a valid place to print a comma:
                currentNode IS NOT a control node
                v
        { key:value, key:value }
                      ^
                    nextNode IS a key node

    There's one problem left, a nNodeList is a node that contains a pointer to
    another list of nodes, which is seamlessly included into the current node
    list. This is great feature when you're writing the JSON definitions, but it
    makes comma printing slightly harder. Here's the revised pseudo-C rules:
        if(node != nControl && ((nextNode == nKey) || (nodeAfterNext == nKey))){
            print_comma();
        }

    This operation involves a pretty crunchy multiple dereference, so that's why
    the actual function has an extra pointer named 'lookAhead'.

    You might notice that the print_comma_if_needed() only has one argument,
    nextNode. It only knows about the NEXT node, but our rules mention the
    CURRENT node too! What gives?
*/