#ifndef _JSON_NODE_H_
#define _JSON_NODE_H_

/* ************************************************************************** */
/*  JSON node type

    The contents of a json_node_t(see below) are in a void pointer, and can't be
    correctly deferenced without knowing what type to expect. This enum contains
    all the possible types you can expect in a node.

    [*] these types aren't currently implemented, sorry
*/
typedef enum {
    nControl,  //
    nFunction, //
    nNodeList, //
    nObject,   // [*]
    nKey,      //
    nString,   //
    nFloat,    //
    nFloat_p2,  //
    nDouble,   // [*]
    nU8,       // [*]
    nU16,      //
    nU24,      // [*]
    nU32,      //
    nS8,       // [*]
    nS16,      // [*]
    nS24,      // [*]
    nS32,      // [*]
    nBool,     // [*]
    nNull,     // [*]
} node_type_t;

/*  JSON node

    A JSON object is represented as an array of node structs.
*/
typedef struct {
    node_type_t type;
    void *contents;
} json_node_t;

// A pointer to a function that returns a pointer to an array of json_node_t
const typedef json_node_t *(*node_function_ptr_t)(void);

// A struct containing ^^^
typedef struct {
    node_function_ptr_t ptr;
} node_function_t;

/* ************************************************************************** */
/*  JSON Nodes:

    This file describes a C data structure that can be used to represent a JSON
    object so that it can be serialized and printed out a UART. The JSON object
    is described as an array of json_node_t structs, each one corresponding to
    a token in the original JSON object.

    * Let's start with an example. *

    C data that you're trying to print
        const char *billsName = "Bill";
        uint16_t billsAge = 56;

    Example node representation:
        const json_node_t jsonBill[] = {
            {nControl, "{"},       // 2
            {nKey, "name"},        // 3
            {nString, &billsName}, // 4
            {nKey, "age"},         // 5
            {nU16, &billsAge},     // 6
            {nControl, "}"},       // 7
            {nControl, "\e"},      //
        };

        const json_node_t jsonPerson[] = {
            {nControl, "{"},        // 0
            {nKey, "person"},       // 1
            {nNodeList, &jsonBill}, //
            {nControl, "\e"},       // 9
        };

    Resulting JSON object, with each 'token' numbered:
        0 1         2 3       4       5      6  7 8 9
        { "person": { "name": "Bill", "age": 56 } } end

    JSON is a hierarchical key:value format. A 'value' can be a JSON data type,
    but it can also be another key:value pair, or multiple key:value pairs.
    These structures can be nested arbitrarily deep, like this:
        {key:val}
        {key: {key:val}} <- note the double closing braces
        {key: {key:val, key:val}}

    Because of this property, in order to evaluate a node, you need to know
    where that node lives in the overall hierarchy. This hierarchy is described
    explicitly using nControl nodes, and implicitly using nKey nodes.

        * nControl: (a pointer to a C string literal)
            Control nodes provide contextual information about the structure of
            the JSON object being printed. Control nodes can indicate one of
            several different actions:
                "{"  - prints a '{', starting a new object
                "}"  - prints a '}', ending the current object
                "\e" - the end of the node array

            Now, I've actually played a trick on you. Go back and reread the
            example, matching each token in the output to the node that
            generates it. Don't worry, I'll wait.

            Find it?

            Token 8 in the output doesn't have a node. Don't panic, your JSON
            serializer hasn't developed sentience(yet), it just keeps track of
            every time it prints a '{' or '}', and when if there are open braces
            that don't have matching closing braces by the time it hits the "\e"
            control node, it'll produce them automatically. This feature makes
            it very slightly more pleasant to write short JSON definitions.

        * nKey: (a pointer to a C string literal)
            An nKey node is identical to an nString(described later) node,
            except that when evaluated, it's followed by a colon (':').

    There's an additional special node type, that can be used to "factor out" a
    section of a node array. This can be used if you feel that your JSON
    description is getting too long to read clearly, or if you have multiple
    JSON descriptions that contain identical sections and you don't want to
    repeat yourself.

        * nNodeList: (a pointer to another array of nodes)
            This node is a pointer to another array of nodes, which are simply
            inserted in place of the current node. This insertion process
            doesn't add any additional JSON punctuation, so if you want the
            included nodes to be in their own object, you'll need to explicitly
            add nControl nodes describing the structure.

    The remaining node types don't have any special behaviour, they're simply a
    reference to some C data. There are node types that correspond to each of
    the JSON data types.
        number  - nFloat, nDouble, nUxx, nSxx
        string  - nString
        boolean - nBool[*]
        array   - nArray[*]
        object  - nObject[*]
        null    - nNull[*]

    [*] This node type isn't implemented, yet. Sorry.

        * nFloat, nDouble, nUxx, nSxx: (a pointer to a C 'number')
            JSON doesn't understand different 'types' of numbers because it's
            derived from Javascript, which is dynamicly and weakly typed.

            C is statically typed and quite particular, because in order to
            interact with a variable(especially via a pointer), it needs to know
            how much memory it occupies and how to interpret that memory.

            Because the contents of a node are stored as a void pointer,
            accessing those contents requires casting the pointer to a
            particular type so that C knows how to access it. JSON's attitude of
            it's a number, lol" isn't going to be sufficient. Therefore, we need
            a node type for each of the C primitive data types:
                floats        - nFloat, nDouble
                unsigned ints - nU8, nU16, nU24, nU32
                signed ints   - nS8, nS16, nS24, nS32

        * nString: (a pointer a C string literal)
            JSON strings are delimited by double quotes ('"'). Since it would be
            really annoying to require every string literal to include it's own
            escaped quotes, the printer handles adding those quotes for you.

        * nBool:
            I'm not sure how you'd really have a pointer a 1 bit bool, so this
            node type will need some additional work to make it functional.

        * nArray:
            This is quite a bit more complicated that a simple data type. An
            array of primitives would require defining the length of the array
            and the data type of its elements. This could be handled by making
            the nArray node point to some kind of struct containing the length,
            type, and a pointer to the actual array to be printed.'

            I'm totally stumped on how I'd represent an array of JSON objects,
            which is a very common

        * nObject:
            Most of the reason to use this JSON type is handled by using control
            nodes.

        * nNull:
            This one's pretty easy, it just prints "null" with no quotes.
*/

/* ************************************************************************** */
/*  JSON array

    A JSON node only contains a single void pointer, which isn't enough
    information to properly reference an array in C. You also need to know the
    length of the array, the type of its elements, and the location of the array
    itself.

    node_type_t contains several values that don't make sense in this context,
    but since it's defined right there(^), we'll use it anyways.

    This feature actually isn't implemented yet, but I'm leaving this here as a
    starting point for when I do.
*/
typedef struct {
    int length;
    node_type_t type;
    void *array;
} json_array_t;

/* ************************************************************************** */

#endif // _JSON_NODE_H_