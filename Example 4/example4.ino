/* ************************************************************************************************************
 * FlashTools - Example program.                                             
 * Example program showing the template features of the write/read functions and saving some more complicated
 * data structures to flash.
 * ***********************************************************************************************************/

#include "FlashTools.h"
#include <string>
#include <Arduino.h>
#include <string.h>

/*
 * Macros for easy printing:
 */
#define _print_0()          SerialUSB.print("")
#define _print_1(s)         SerialUSB.print(s)
#define _print_2(s, t)      _print_1(s),    _print_1(t)
#define _print_3(s, t, u)   _print_2(s, t), _print_1(u)

#define _println_0()        SerialUSB.println()
#define _println_1(s)       SerialUSB.println(s)
#define _println_2(s, t)    _println_1(s),    _println_1(t)
#define _println_3(s, t, u) _println_2(s, t), _println_1(u)

#define _print_x(x, s, t, u, FUNC, ...)    FUNC
#define _println_x(x, s, t, u, FUNC, ...)  FUNC

#define _print(...) _print_x(, ##__VA_ARGS__ ,  \
                        _print_3(__VA_ARGS__),  \
                        _print_2(__VA_ARGS__),  \
                        _print_1(__VA_ARGS__),  \
                        _print_0(__VA_ARGS__)   )     

#define _println(...) _println_x(, ##__VA_ARGS__ ,  \
                          _println_3(__VA_ARGS__),  \
                          _println_2(__VA_ARGS__),  \
                          _println_1(__VA_ARGS__),  \
                          _println_0(__VA_ARGS__)   )    
  
#define _println_str(s) _println(s.c_str())
#define _println_hex(s) SerialUSB.println(s, HEX)

#define new_str(s) "New Test String "#s

/* 
 * Array sizes:
 */
#define SIZE1 10
#define SIZE2 512

/*
 * To remove an example, delete/comment its corresponding macro:
 */
#define EXAMPLE1 1
#define EXAMPLE2 1
#define EXAMPLE3 1
#define EXAMPLE4 1

/*
 * Struct declarations for the structures (examples 1 & 2):
 */
#if EXAMPLE1
struct Test_Struct {
    int value1;
    double value2;
    byte value3;
    char *message;
    std::string *str_arr;
};
#endif

#if EXAMPLE2
struct Person {
    std::string name;
    char gender[7];
    unsigned int age;
};
#endif

/*
 * Template struct declaration for linked list node (example 4):
 */
#if EXAMPLE4
template <typename T>
struct Node {
    T data;
    Node<T> *next;
    Node<T>(T val = 0, Node<T> *link = NULL): data(val), next(link) {};
};
#endif

/*
 * Setup:
 */
void setup() {
    delay(6000);
    SerialUSB.begin(9600);
    SerialUSB.print("Example Start\n");
}

/*
 * Main:
 */
void loop() {
    
    /* ~~~ Instantiate dueEFC object ~~~ */
    FlashTools flash1;
    
    /* ~~~ Declare & initialize variables for each example ~~~ */
    
#if EXAMPLE1
    /*** Example 1 Variables ***/
    uint32_t flash_addr1 = flash1.getPageAddress(1025);
    
    /* Create a test struct to be written to flash and declare another to hold data copied from flash */
    Test_Struct struct_data {-11656, 18888888.888, 0, "Hello, world!", new std::string[5]};
    Test_Struct copied_struct_data1;
    
    /* Declare an array of bytes to hold copied test struct data */
    byte byte_array[sizeof(struct_data)];
    Test_Struct *copied_struct_data2 = reinterpret_cast<Test_Struct *>(byte_array);
    
    /* Initialize the string array member of our test struct */
    struct_data.str_arr[0] = new_str(1);
    struct_data.str_arr[1] = new_str(2);
    struct_data.str_arr[2] = new_str(3);
    struct_data.str_arr[3] = new_str(4);
    struct_data.str_arr[4] = new_str(5);
    
#endif
        
#if EXAMPLE2
    /*** Example 2 Variables ***/
    Person *flash_addr2 = flash1.getAddress<Person>(1060);
    
    /* Names / ages for struct array */
    char *names[10] {"Alex A.", "Bob B.", "Chris C.", "Dave D.", "Evan E.", "Ginger G.", "Jessica J.", "Katie K.", "Maria M.", "Stephanie S."};
    unsigned int ages[10] {16, 19, 66, 24, 8, 25, 72, 21, 18, 30};
    
    /* Person struct array to be written to flash and another to hold data copied from flash */
    Person person_arr[SIZE1];
    Person person_arr_copied[SIZE1];
    
    /* Additional Person structs and byte array for additional example */
    Person single_person1, single_person2, single_person3;
    byte byte_array2[sizeof(Person) * SIZE1];
    Person *person_arr_copied2 = reinterpret_cast<Person *>(byte_array2);
    
    /* Assign values to each person struct in array */
    for (int i = 0; i < SIZE1; ++i) {
        person_arr[i].name   = names[i];
        strcpy(person_arr[i].gender, i < 5 ? "Male" : "Female");
        person_arr[i].age    = ages[i];
    }
#endif

#if EXAMPLE3
    /*** Example 3 Variables ***/
    uint32_t *flash_addr3 = flash1.getAddress<uint32_t>(1090, 12);
    
    /* Create uint32_t array to be written to flash and another to store data read from flash */
    uint32_t numbers[SIZE2];
    uint32_t read_numbers[SIZE2];
    for (uint32_t i {0}; i < SIZE2; ++i) {
        numbers[i] = i * 3;
    }
#endif
    
#if EXAMPLE4
    /*** Example 4 Variables ***/
    Node<uint32_t> *flash_addr4 = flash1.getAddress< Node<uint32_t> >(1201);
    
    /* Hexadecimal values to be stored in linked list */
    uint32_t hex_values[SIZE1] {0xDF35B371, 0xE0FF7BF1, 0x3DB6115D, 0xFBBFBCCC, 0xDDF69E06, 0x5D470843, 0xA3BDAE71, 0xFE3FFF66, 0xCFFFFB83, 0xFFBBFF0B}, idx {0};

    /* Create a linked list using the Node struct */
    Node<uint32_t> *root = NULL;
    for (Node<uint32_t> **p_node = &root; idx < SIZE1; ++idx) {
        *p_node = new Node<uint32_t>(hex_values[idx],NULL);
        p_node = &(*p_node)->next;
    }
#endif

    /* ~~~ Write data to flash & copy or read data from flash ~~~ */

#if EXAMPLE1
    /* Write our test struct to the first flash address */
    if (flash1.write<Test_Struct>(flash_addr1, &struct_data, sizeof(struct_data))) {
        _println("Error! Test Struct flash write was not successful.");
    }
    
    /* Copy the data from the first flash address to copied_struct_data1 */
    if (flash1.copy<Test_Struct>(flash_addr1, &copied_struct_data1, sizeof(struct_data)) == NULL) {
        _println("Error! Test Struct copy from flash (#1) was not successful.");
    }
    /* Copy the data from the first flash address to the array of bytes */
    if (flash1.copy<byte>(flash_addr1, byte_array, sizeof(struct_data)) == NULL) {
        _println("Error! Test Struct copy from flash (#2) was not successful.");
    }
#endif
        
#if EXAMPLE2
    /* Write the array of Person structs to second flash address */
    if (flash1.write<Person>(flash_addr2, person_arr, sizeof(Person) * SIZE1)) {
        _println("Error! Person struct array flash write was not successful.");
    }
    
    /* Copy data from flash to the second Person struct array */
    if (flash1.copy<Person>(flash_addr2, person_arr_copied, sizeof(Person) * SIZE1) == 0) {
        _println("Error! Person struct array copy from flash (#1) was not successful.");
    }
    /* Copy data from flash to the second byte array */
    if (flash1.copy<byte>(reinterpret_cast<byte *>(flash_addr2), byte_array2, sizeof(Person) * SIZE1) == NULL) {
        _println("Error! Person struct array copy from flash (#2) was not successful.");
    }
    
    /* Copy data from flash to the single Person structs */
    if (flash1.copy<Person>(reinterpret_cast<Person *>(flash_addr2), &single_person1, sizeof(person_arr[0])) == 0) {
        _println("Error! Person struct copy (#1) from flash was not successful.");
    }
    if (flash1.copy<Person>(reinterpret_cast<Person *>(flash_addr2) + 1, &single_person2, sizeof(person_arr[1])) == 0) {
        _println("Error! Person struct copy (#2) from flash was not successful.");
    }
    if (flash1.copy<Person>(reinterpret_cast<Person *>(flash_addr2) + 2, &single_person3, sizeof(person_arr[2])) == 0) {
        _println("Error! Person struct copy (#3) from flash was not successful.");
    }
#endif
        
#if EXAMPLE3
    /* Write the array of numbers to the third flash address */
    if (flash1.write<uint32_t>(flash_addr3, numbers, SIZE2 * sizeof(uint32_t))) {
        _print("Error! Dword array flash write was not successful.");
    }
    
    /* Read from third flash address, one dword at a time, and save to read_numbers */
    for (uint32_t idx {0}; idx < SIZE2; ++idx) {
        read_numbers[idx] = flash1.read<uint32_t>(flash_addr3 + idx);
    }
#endif

#if EXAMPLE4
    /* Write each node to the fourth flash address. Delete each node after writing it */
    for (Node<uint32_t> *node = root, *p_addr = flash_addr4, *hold; node != NULL; hold = node->next, delete node, node = hold) {
        if (flash1.write< Node<uint32_t> >(p_addr++, node, sizeof(Node<uint32_t>))) {
            _println("Error! Linked list node write to flash was not successful.");
        }
    }
    
    /* Copy data from flash to newly allocated nodes */
    Node<uint32_t> *new_root = NULL;
    for (Node<uint32_t> **p_node = &new_root, *p_addr = flash_addr4; *p_node != NULL || p_addr == flash_addr4; p_node = &(*p_node)->next) {
        *p_node = new Node<uint32_t>(0, NULL);
        if (flash1.copy< Node<uint32_t> >(p_addr++, *p_node, sizeof(Node<uint32_t>)) == NULL) {
            _println("Error! Linked list node copy from flash was not successful.");
        }
    }
#endif


    /* ~~~ Output copied/read data ~~~ */
    
    _println("--- Flash operations complete ---");
    _println("--- Now displaying data ---");
    
    while (true) {
#if EXAMPLE1
        _println(" ", "Test Struct copied data (#1): ");
        _println("-----------------------------"," ");
        
        _println(copied_struct_data1.value1, copied_struct_data1.value2, copied_struct_data1.value3);
        _println(copied_struct_data1.message);
        
        for (uint32_t i {0}; i < 5; ++i) {
            _println_str(copied_struct_data1.str_arr[i]);
        }
        _println();
        _println();
        
        _println("Test Struct copied data (#2): ");
        _println("-----------------------------"," ");

        _println(copied_struct_data2->value1, copied_struct_data2->value2, copied_struct_data2->value3);
        _println(copied_struct_data2->message);
        
        for (uint32_t i {0}; i < 5; ++i) {
            _println_str(copied_struct_data2->str_arr[i]);
        }
      
        _println("\n");
#endif
        
#if EXAMPLE2
        _println("Copied Person Struct Array data (#1): ");
        _println("-----------------------------"," ");

        for (uint32_t i {0}; i < SIZE1; ++i) {
            _print("Person ", i + 1, " data:\n");
            _println_str(person_arr_copied[i].name);
            _println(person_arr_copied[i].gender, person_arr_copied[i].age, " ");
        }
        _println();
        
        _println("Copied Person Struct Array data (#2): ");
        _println("-----------------------------"," ");
        
        for (uint32_t i {0}; i < SIZE1; ++i) {
            _print("Person ", i + 1, " data:\n");
            _println_str(person_arr_copied2[i].name);
            _println(person_arr_copied2[i].gender, person_arr_copied2[i].age, " ");
        }
        _println();
        
        _println("Copied Person Struct data (#1): ");
        _println("-----------------------------"," ");
        _println_str(single_person1.name);
        _println(single_person1.gender, single_person1.age, " ");
        
        _println("Copied Person Struct data (#2): ");
        _println("-----------------------------"," ");
        _println_str(single_person2.name);
        _println(single_person2.gender, single_person2.age, " ");
        
        _println("Copied Person Struct data (#3): ");
        _println("-----------------------------"," ");
        _println_str(single_person3.name);
        _println(single_person3.gender, single_person3.age, " ");
        _println("\n");
#endif
        
#if EXAMPLE3
        _println("Read numbers (uint32_t) array data: ");
        _println("-----------------------------"," ");
        
        for (int i {0}; i < SIZE2; ++i) {
            _println(read_numbers[i]);
        }
         _println("\n");
#endif

#if EXAMPLE4
        _println("Copied Linked List Data: ");
        _println("-----------------------------"," ");

        for (Node<uint32_t> *cur = new_root; cur != NULL; cur = cur->next) {
            _println_hex(cur->data);
            _print("Linked address (next): ", cur->next ? reinterpret_cast<uintptr_t>(cur->next) : 0, "\n");
        }
        _println("\n");
#endif

        _println("--- Finished displaying all data. Now entering delay. ---");
        delay(10000);
    }   
}
