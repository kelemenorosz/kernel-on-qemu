#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "print.h"

#define G_HEAP_PAGE_SIZE 0x1000

#define G_HEAP_MAX_BLOCK_SIZE 0x20000 // 128K

extern uint32_t* const g_memoryMapPtr; 
uint8_t* g_heapStart 	= 0x0;
uint32_t g_pageCount 	= 0x0;

void* g_heapStart_buddy = NULL;
void* g_heap_body_buddy = NULL;

typedef struct __attribute__((__packed__)) ADDRESS_RANGE_DESCRIPTOR {

	uint32_t baseL;
	uint32_t baseH;
	uint32_t lengthL;
	uint32_t lengthH;
	uint32_t type;
	uint32_t acpi;

} ADDRESS_RANGE_DESCRIPTOR;

typedef struct __attribute__((__packed__)) BUDDY_NODE {

	uint8_t type;
	uint8_t status;
	uint8_t sidedness;
	uint8_t padding;
	void* address;
	struct BUDDY_NODE* list_next;
	struct BUDDY_NODE* list_prev;
	struct BUDDY_NODE* leaf_left;
	struct BUDDY_NODE* leaf_right;

} BUDDY_NODE;

#define BUDDY_NODE_TYPE_4K 		0b00000001
#define BUDDY_NODE_TYPE_8K 		0b00000010
#define BUDDY_NODE_TYPE_16K 	0b00000100
#define BUDDY_NODE_TYPE_32K 	0b00001000
#define BUDDY_NODE_TYPE_64K 	0b00010000
#define BUDDY_NODE_TYPE_128K 	0b00100000

#define BUDDY_NODE_STATUS_ALLOCATED 	0x0
#define BUDDY_NODE_STATUS_UNALLOCATED 	0x1
#define BUDDY_NODE_STATUS_NONODE	 	0x2

#define BUDDY_NODE_SIDEDNESS_LEFT		0x0
#define BUDDY_NODE_SIDEDNESS_RIGHT		0x1
#define BUDDY_NODE_SIDEDNESS_NONE		0x2

typedef struct __attribute__((__packed__, __aligned__(4))) BUDDY_LISTS {

	BUDDY_NODE* node4k_first;
	BUDDY_NODE* node8k_first;
	BUDDY_NODE* node16k_first;
	BUDDY_NODE* node32k_first;
	BUDDY_NODE* node64k_first;
	BUDDY_NODE* node128k_first;

} BUDDY_LISTS;

BUDDY_LISTS g_buddy_lists = {};

void merge_bnode(BUDDY_NODE* bn, size_t limit);
void merge_bnode_nolimit(BUDDY_NODE* bn);
void init_bnode(BUDDY_NODE* bn, uint8_t status, uint8_t type, uint8_t sidedness, void* address, BUDDY_NODE* list_next, BUDDY_NODE* list_prev, BUDDY_NODE* leaf_left, BUDDY_NODE* leaf_right);

void memset(void* ptr, uint8_t value, size_t num) {

	uint8_t* ptr_cast = ptr;
	for (size_t i = 0; i < num; ++i) {

		*ptr_cast = value;
		++ptr_cast;

	}

	return;

}

void memcpy(void* destination, const void* source, size_t num) {

	uint8_t* destination_cast = destination;
	const uint8_t* source_cast = source;

	for (size_t i = 0; i < num; ++i) {

		*destination_cast = *source_cast;

		++destination_cast;
		++source_cast;

	}

	return;

}

/* 
Function: 		memory_init
Description: 	Initialize heap from memory map information.
				Allocate memory space for buddy allocator header.
				Initializes the buddy allocator highest order block nodes. At the moment it is set to 128K.
Return:			NONE
*/
void memory_init() {

	uint16_t memory_map_length	= *((uint16_t*)g_memoryMapPtr) - 0x2;
	uint16_t memory_map_count 	= memory_map_length / sizeof(ADDRESS_RANGE_DESCRIPTOR);

	uint8_t* memmap_ptr = ((uint8_t*)g_memoryMapPtr) + 0x2;
	uint32_t maxLen = 0x0;

	for (size_t i = 0; i < memory_map_count; ++i) {
		ADDRESS_RANGE_DESCRIPTOR* addr_range_desc = (ADDRESS_RANGE_DESCRIPTOR*)memmap_ptr;
		if (addr_range_desc->lengthL > maxLen && addr_range_desc->type == 0x1) {
			g_heapStart = (uint8_t*)addr_range_desc->baseL;
			g_heapStart_buddy = (uint8_t*)addr_range_desc->baseL;
			maxLen = addr_range_desc->lengthL;
		}
		memmap_ptr += sizeof(ADDRESS_RANGE_DESCRIPTOR);
	}

	// -- Calculate how many max blocks are there
	uint32_t max_block_count_re = maxLen / (sizeof(BUDDY_NODE) * 0x3F + G_HEAP_MAX_BLOCK_SIZE); // 0x3F is the number of nodes per max blocks when G_HEAP_MAX_BLOCK_SIZE is 128K

	uint32_t buddy_header_size = sizeof(BUDDY_NODE) * 0x3F * max_block_count_re; 
	uint32_t buddy_header_mod = buddy_header_size % G_HEAP_MAX_BLOCK_SIZE;

	if (buddy_header_mod != 0) {
		buddy_header_size = buddy_header_size / G_HEAP_MAX_BLOCK_SIZE;
		buddy_header_size++;
		buddy_header_size *= G_HEAP_MAX_BLOCK_SIZE;
	} 
	else {
		buddy_header_size /= G_HEAP_MAX_BLOCK_SIZE;
		buddy_header_size *= G_HEAP_MAX_BLOCK_SIZE;
	}

	// -- Initialize buddy memory allocator

	/*
	First allocate space for the 512K memory chunks.
	Leave the first 512K block for the buddy allocator.
	*/

	void* buddy_body_start = g_heapStart_buddy + buddy_header_size;

	g_buddy_lists.node4k_first = NULL;
	g_buddy_lists.node8k_first = NULL;
	g_buddy_lists.node16k_first = NULL;
	g_buddy_lists.node32k_first = NULL;
	g_buddy_lists.node64k_first = NULL;
	g_buddy_lists.node128k_first = NULL;

	// -- Initialize the highest order nodes. Add them to the free lists

	BUDDY_NODE* prev_node = NULL;
	for (size_t i = 0; i < max_block_count_re; ++i) {
		BUDDY_NODE* node = g_heapStart_buddy + i * (63) * sizeof(BUDDY_NODE);
		node->type = BUDDY_NODE_TYPE_128K;
		node->status = BUDDY_NODE_STATUS_UNALLOCATED;
		node->sidedness = BUDDY_NODE_SIDEDNESS_NONE;
		node->padding = 0x0;
		node->address = (void*)(buddy_body_start + i * G_HEAP_MAX_BLOCK_SIZE);
		if (prev_node != NULL) prev_node->list_next = node;
		node->list_next = NULL;
		node->leaf_left = NULL;
		node->leaf_right = NULL;
		node->list_prev = prev_node;
		prev_node = node;

	}

	g_buddy_lists.node128k_first = (BUDDY_NODE*)g_heapStart_buddy;

	// -- Set the heap body start address
	g_heap_body_buddy = buddy_body_start;

	return;

}

/*
Function:		kalloc
Description: 	Allocates 'page_count' contiguous pages.
Return:			Pointer to first page start.
*/
void* kalloc(size_t page_count) {

	uint32_t page_count_po2 = page_count;

	page_count_po2--;
	page_count_po2 |= page_count_po2 >> 1;
	page_count_po2 |= page_count_po2 >> 2;
	page_count_po2 |= page_count_po2 >> 4;
	page_count_po2 |= page_count_po2 >> 8;
	page_count_po2 |= page_count_po2 >> 16;
	page_count_po2++;

	uint32_t block_level = page_count_po2;

	BUDDY_NODE** buddy_list_check_start = (BUDDY_NODE**)&g_buddy_lists.node4k_first;
	uint32_t buddy_list_check_start_index = 0x0;
	while (page_count_po2 != 1) {
		page_count_po2 >>= 1;
		buddy_list_check_start_index++;
	} 
	buddy_list_check_start += buddy_list_check_start_index;

	while (*buddy_list_check_start == NULL) {
		buddy_list_check_start++;
		block_level <<= 1;
	}

	BUDDY_NODE* bn_free = (*buddy_list_check_start);
	// -- Check if it is not max block

	page_count_po2 = page_count;

	page_count_po2--;
	page_count_po2 |= page_count_po2 >> 1;
	page_count_po2 |= page_count_po2 >> 2;
	page_count_po2 |= page_count_po2 >> 4;
	page_count_po2 |= page_count_po2 >> 8;
	page_count_po2 |= page_count_po2 >> 16;
	page_count_po2++;

	if (block_level == 0x20) {
		while (bn_free->status == BUDDY_NODE_STATUS_ALLOCATED) bn_free = bn_free->list_next; 
	}

	// -- Mark it as used

	bn_free->status = BUDDY_NODE_STATUS_ALLOCATED;

	if (block_level != 0x20) {
		*buddy_list_check_start = bn_free->list_next;
		(*buddy_list_check_start)->list_prev = NULL;
		bn_free->list_next = NULL;
		bn_free->list_prev = NULL;
	}

	// -- Split

	uint32_t split_size = page_count; 
	uint32_t initial_size = block_level;

	BUDDY_NODE* split_node = bn_free;

	uint32_t leaf_space = 0x0;
	uint32_t leaf_space_help = block_level >> 1;
	while (leaf_space_help > 0) {
		leaf_space += leaf_space_help;
		leaf_space_help >>= 1; 
	}

	while (split_size != 0) {

		BUDDY_NODE* leaf_left = split_node + 1;
		BUDDY_NODE* leaf_right = leaf_left + leaf_space;

		split_node->leaf_left = leaf_left;
		split_node->leaf_right = leaf_right;

		buddy_list_check_start--;

		if (split_size == initial_size) {

			split_size = 0;

			split_node->leaf_left = NULL;
			split_node->leaf_right = NULL;

		}
		else if (split_size > initial_size / 2) {

			init_bnode(leaf_left, BUDDY_NODE_STATUS_ALLOCATED, split_node->type / 2, BUDDY_NODE_SIDEDNESS_LEFT, split_node->address, NULL, NULL, NULL, NULL);
			init_bnode(leaf_right, BUDDY_NODE_STATUS_ALLOCATED, split_node->type / 2, BUDDY_NODE_SIDEDNESS_RIGHT, split_node->address + split_node->type * G_HEAP_PAGE_SIZE / 2, NULL, NULL, NULL, NULL);
			
			split_size -= initial_size / 2;
			split_node = leaf_right;

		}
		else if (split_size == initial_size / 2) {

			init_bnode(leaf_left, BUDDY_NODE_STATUS_ALLOCATED, split_node->type / 2, BUDDY_NODE_SIDEDNESS_LEFT, split_node->address, NULL, NULL, NULL, NULL);
			init_bnode(leaf_right, BUDDY_NODE_STATUS_UNALLOCATED, split_node->type / 2, BUDDY_NODE_SIDEDNESS_RIGHT, split_node->address + split_node->type * G_HEAP_PAGE_SIZE / 2, NULL, NULL, NULL, NULL);

			leaf_right->list_next = *buddy_list_check_start;
			(*buddy_list_check_start)->list_prev = leaf_right;
			*buddy_list_check_start = leaf_right;

			split_node = leaf_left;
			split_size = 0;

		}
		else {

			init_bnode(leaf_left, BUDDY_NODE_STATUS_ALLOCATED, split_node->type / 2, BUDDY_NODE_SIDEDNESS_LEFT, split_node->address, NULL, NULL, NULL, NULL);
			init_bnode(leaf_right, BUDDY_NODE_STATUS_UNALLOCATED, split_node->type / 2, BUDDY_NODE_SIDEDNESS_RIGHT, split_node->address + split_node->type * G_HEAP_PAGE_SIZE / 2, NULL, NULL, NULL, NULL);

			leaf_right->list_next = *buddy_list_check_start;
			(*buddy_list_check_start)->list_prev = leaf_right;
			*buddy_list_check_start = leaf_right;

			split_node = leaf_left;

		}

		initial_size >>= 1;
		leaf_space >>= 1;

	}

	return bn_free->address;

}

/*
Function:		kfree
Description: 	Frees 'page_count' pages starting from address 'address'.
Return:			NONE
*/
void kfree(void* address, size_t page_count) {

	// -- Get the highest order address

	uint32_t* highest_order_address = (uint32_t*)((uint32_t)address / G_HEAP_MAX_BLOCK_SIZE);
	highest_order_address = (uint32_t*)((uint32_t)highest_order_address * G_HEAP_MAX_BLOCK_SIZE);

	BUDDY_NODE* high_order_node = g_buddy_lists.node128k_first;
	while (high_order_node->address != highest_order_address) high_order_node = high_order_node->list_next;

	// -- Find root node
	BUDDY_NODE* root_bn = high_order_node;
	while (root_bn->address < address) {
		if (root_bn->leaf_right->address > address) {
			root_bn = root_bn->leaf_left;	
		}
		else {
			root_bn = root_bn->leaf_right;	
		}
	}

	merge_bnode(root_bn, page_count);
	merge_bnode_nolimit(high_order_node);

	// Put root onto free lists if it was not merged in merge_bnode_nolimit() and is not max block
	if (root_bn != high_order_node && root_bn->status == BUDDY_NODE_STATUS_UNALLOCATED) {

		BUDDY_NODE** free_lists = (BUDDY_NODE**)&g_buddy_lists.node4k_first;
		uint32_t order = root_bn->type;
		uint32_t free_list_index = 0x0;
		while (order != 1) {
			order >>= 1;
			free_list_index++;
		}
		free_lists += free_list_index;		

		root_bn->list_next = *free_lists;
		(*free_lists)->list_prev = root_bn;		
		*free_lists = root_bn;

	}

	return;

}


/*
Function:		merge_bnode_nolimit
Description: 	Merges bnodes.
Return:			NONE
*/
void merge_bnode_nolimit(BUDDY_NODE* bn) {

	if (bn->leaf_left && bn->leaf_right) {
		if (bn->leaf_left) {
			merge_bnode_nolimit(bn->leaf_left);
		}
		if (bn->leaf_right) {
			merge_bnode_nolimit(bn->leaf_right);
		}

		if (bn->leaf_left->status == BUDDY_NODE_STATUS_UNALLOCATED && bn->leaf_right->status == BUDDY_NODE_STATUS_UNALLOCATED) {

			BUDDY_NODE** free_lists = (BUDDY_NODE**)&g_buddy_lists.node4k_first;
			uint32_t order = bn->leaf_left->type;
			uint32_t free_list_index = 0x0;
			while (order != 1) {
				order >>= 1;
				free_list_index++;
			}
			free_lists += free_list_index;

			if (bn->leaf_left->list_next != NULL || bn->leaf_left->list_prev != NULL || *free_lists == bn->leaf_left) {
				if (*free_lists == bn->leaf_left) {
					*free_lists = bn->leaf_left->list_next;
				}
				if (bn->leaf_left->list_prev != NULL) bn->leaf_left->list_prev->list_next = bn->leaf_left->list_next;
				if (bn->leaf_left->list_next != NULL) bn->leaf_left->list_next->list_prev = bn->leaf_left->list_prev;
				bn->leaf_left->list_next = NULL;
				bn->leaf_left->list_prev = NULL;
			}

			if (bn->leaf_right->list_next != NULL || bn->leaf_right->list_prev != NULL || *free_lists == bn->leaf_right) {
				if (*free_lists == bn->leaf_right) {
					*free_lists = bn->leaf_right->list_next;
				}
				if (bn->leaf_right->list_prev != NULL) bn->leaf_right->list_prev->list_next = bn->leaf_right->list_next;
				if (bn->leaf_right->list_next != NULL) bn->leaf_right->list_next->list_prev = bn->leaf_right->list_prev;
				bn->leaf_right->list_next = NULL;
				bn->leaf_right->list_prev = NULL;
			}

			bn->leaf_left->status = BUDDY_NODE_STATUS_NONODE;
			bn->leaf_right->status = BUDDY_NODE_STATUS_NONODE;
			bn->leaf_left = NULL;
			bn->leaf_right = NULL;
			bn->status = BUDDY_NODE_STATUS_UNALLOCATED;
		}

		return;

	}
	else {

		return;
	
	}

}

/*
Function:		merge_bnode
Description: 	Merges bnodes.
Return:			NONE
*/
void merge_bnode(BUDDY_NODE* bn, size_t limit) {

	// All nodes that are called by merge_bnode are allocated

	// If the node has any leaves
	if (bn->leaf_left != NULL && bn->leaf_right != NULL) {

		// Left node, if it exists is always 
		if (bn->leaf_left != NULL) {
			merge_bnode(bn->leaf_left, limit);
		}
		if (bn->leaf_right != NULL && limit > bn->type / 2) {
			merge_bnode(bn->leaf_right, limit - bn->type / 2);
		}

		BUDDY_NODE** free_lists = (BUDDY_NODE**)&g_buddy_lists.node4k_first;
		uint32_t order = bn->leaf_left->type;
		uint32_t free_list_index = 0x0;
		while (order != 1) {
			order >>= 1;
			free_list_index++;
		}
		free_lists += free_list_index;

		// If both leaves are unallocated declare parent as unallocated
		if(bn->leaf_left->status == BUDDY_NODE_STATUS_UNALLOCATED && bn->leaf_right->status == BUDDY_NODE_STATUS_UNALLOCATED) {

			if (bn->leaf_left->list_next != NULL || bn->leaf_left->list_prev != NULL || *free_lists == bn->leaf_left) {
				if (*free_lists == bn->leaf_left) {
					*free_lists = bn->leaf_left->list_next;
				}
				if (bn->leaf_left->list_prev != NULL) bn->leaf_left->list_prev->list_next = bn->leaf_left->list_next;
				if (bn->leaf_left->list_next != NULL) bn->leaf_left->list_next->list_prev = bn->leaf_left->list_prev;
				bn->leaf_left->list_next = NULL;
				bn->leaf_left->list_prev = NULL;
			}

			if (bn->leaf_right->list_next != NULL || bn->leaf_right->list_prev != NULL || *free_lists == bn->leaf_right) {
				if (*free_lists == bn->leaf_right) {
					*free_lists = bn->leaf_right->list_next;
				}
				if (bn->leaf_right->list_prev != NULL) bn->leaf_right->list_prev->list_next = bn->leaf_right->list_next;
				if (bn->leaf_right->list_next != NULL) bn->leaf_right->list_next->list_prev = bn->leaf_right->list_prev;
				bn->leaf_right->list_next = NULL;
				bn->leaf_right->list_prev = NULL;
			}

			bn->leaf_left = NULL;
			bn->leaf_right = NULL;
			bn->status = BUDDY_NODE_STATUS_UNALLOCATED;
		}
		// Set, if any leaves were freed but not both, onto the free lists
		else {

			if (bn->leaf_left->status == BUDDY_NODE_STATUS_UNALLOCATED && (bn->leaf_left->list_next != NULL || bn->leaf_left->list_prev != NULL)) {
				bn->leaf_left->list_next = *free_lists;
				(*free_lists)->list_prev = bn->leaf_left;
				*free_lists = bn->leaf_left;
			}
			if (bn->leaf_right->status == BUDDY_NODE_STATUS_UNALLOCATED && (bn->leaf_right->list_next != NULL || bn->leaf_right->list_prev != NULL)) {
				bn->leaf_right->list_next = *free_lists;
				(*free_lists)->list_prev = bn->leaf_right;
				*free_lists = bn->leaf_right;
			}

		}

		return;

	} 
	// No leaves
	else {

		bn->status = BUDDY_NODE_STATUS_UNALLOCATED;
		return;

	}

}


/*
Function:		init_bnode
Description: 	Initializes a buddy allocator node.
Return:			NONE
*/
void init_bnode(BUDDY_NODE* bn, uint8_t status, uint8_t type, uint8_t sidedness, void* address, BUDDY_NODE* list_next, BUDDY_NODE* list_prev, BUDDY_NODE* leaf_left, BUDDY_NODE* leaf_right) {

	bn->status = status;
	bn->type = type;
	bn->sidedness = sidedness;
	bn->address = address;
	bn->list_next = list_next;
	bn->list_prev = list_prev;
	bn->leaf_left = leaf_left;
	bn->leaf_right = leaf_right;
	
	return;

}

void print_memmap() {

	uint16_t memory_map_length	= *((uint16_t*)g_memoryMapPtr) - 0x2;
	uint16_t memory_map_count 	= memory_map_length / sizeof(ADDRESS_RANGE_DESCRIPTOR);
	
	print_string("Memory map entry count: ");
	print_word(memory_map_count);
	print_newline();

	uint8_t* memmap_ptr = ((uint8_t*)g_memoryMapPtr) + 0x2;

	for (size_t i = 0; i < memory_map_count; ++i) {

		ADDRESS_RANGE_DESCRIPTOR* addr_range_desc = (ADDRESS_RANGE_DESCRIPTOR*)memmap_ptr;
		print_dword(addr_range_desc->baseL);
		print_space(0x1);
		print_dword(addr_range_desc->baseH);
		print_space(0x1);
		print_dword(addr_range_desc->lengthL);
		print_space(0x1);
		print_dword(addr_range_desc->lengthH);
		print_space(0x1);
		print_dword(addr_range_desc->type);
		print_space(0x1);
		print_dword(addr_range_desc->acpi);
		print_space(0x1);

		print_newline();
	
		memmap_ptr += sizeof(ADDRESS_RANGE_DESCRIPTOR);

	}

	return;

}

void print_freelists() {

	print_string("Buddy_list: ");
	print_dword((uint32_t)g_buddy_lists.node4k_first->address);
	print_string(", type: ");
	print_byte(g_buddy_lists.node4k_first->type);
	print_newline();
	print_string("Buddy_list: ");
	print_dword((uint32_t)g_buddy_lists.node8k_first->address);
	print_string(", type: ");
	print_byte(g_buddy_lists.node8k_first->type);
	print_newline();
	print_string("Buddy_list: ");
	print_dword((uint32_t)g_buddy_lists.node16k_first->address);
	print_string(", type: ");
	print_byte(g_buddy_lists.node16k_first->type);
	print_newline();
	print_string("Buddy_list: ");
	print_dword((uint32_t)g_buddy_lists.node32k_first->address);
	print_string(", type: ");
	print_byte(g_buddy_lists.node32k_first->type);
	print_newline();
	print_string("Buddy_list: ");
	print_dword((uint32_t)g_buddy_lists.node64k_first->address);
	print_string(", type: ");
	print_byte(g_buddy_lists.node64k_first->type);
	print_newline();
	print_string("Buddy_list: ");
	print_dword((uint32_t)g_buddy_lists.node128k_first->address);
	print_string(", type: ");
	print_byte(g_buddy_lists.node128k_first->type);
	print_newline();

	return;

}