/* ... */

#include <stdio.h>

#include "query.h"
#include "list.h"
#include "heap.h"

#include <stdlib.h>
#include <assert.h>

// see Assignment spec: 'Task 1: Array-based accumulation approach'


//initialise an array of n_documents
void initialise(int num, float *docs);

// iterate each document list in the index
void ite_data(Index *index, float *docs);

// find the top n_result documents and print them out.
void find_top_score(int size, float *doc_score, int n_documents);

//print out the results
void print_results(Heap *heap, int size,float *doc_score);


/****************************************************************/
//initialise an array of n_documents
void initialise(int num, float *docs){
	int i;
	for (i=0;i<num;i++){
		docs[i] = 0.0;
	}
}

// iterate each document list in the index
void ite_data(Index *index, float *docs){
	int i;
	List *list;
	Node *temp;

	int num_term = index->num_terms;
	List **doclists = index->doclists;
	//access the linked list in index
	for (i=0;i<num_term;i++){
		list = doclists[i]; //is a pointer of linked list
		assert(list!=NULL && list->head!=NULL);
		//read_list(list,docs); // read the data in linked list
		temp = list->head;
		while(temp!=NULL){
			Document *data = temp->data;
			docs[data->id] += data->score;
			temp = temp->next;
		}
	}
}


// find the top n_result documents and print them out.
void find_top_score(int size, float *doc_score, int n_documents){
	int i;
	Heap *top;
	top = new_heap(size);

	// put n_results in the heap
	for (i=0;i<size;i++){
		heap_insert(top,doc_score[i],i);
	}
	for (i=size;i<n_documents;i++){
		if (doc_score[i]>heap_peek_key(top)){
			// if next score is bigger than the min score in the heap
			// put it into the heap
			heap_remove_min(top);
			heap_insert(top,doc_score[i],i);
		}
	}
	print_results(top, size,doc_score);
	free_heap(top);
	

}

//print out the results
void print_results(Heap *heap, int size, float *doc_score){
	int i,id;
	int *id_list = (int*)malloc(sizeof(int)*size);;
	// put the top score document ids in a array
	for (i=0;i<size;i++){
		id_list[size-i-1] = heap_remove_min(heap);
	}
	// print out the results
	for (i=0;i<size;i++){
		id = id_list[i];
		printf("%6d %.6f\n", id, doc_score[id]);
	}
	free(id_list);
	id_list = NULL;


}

/****************************************************************/
void print_array_results(Index *index, int n_results, int n_documents) {
	float *doc_score;
	doc_score = (float*)malloc(sizeof(float)*n_documents);	
	initialise(n_documents, doc_score);
	ite_data(index, doc_score);
	find_top_score(n_results,doc_score,n_documents);
	free(doc_score);
	doc_score = NULL;

}

/****************************************************************/
void creat_heap(Index *index, Heap *heap, Node **array);
void creat_heap(Index *index, Heap *heap, Node **array){
	int num = index->num_terms;
	int i;	
	List **doclists = index->doclists;
	
	// initialise the heap
	for (i=0;i<num;i++){
		Document *data = doclists[i]->head->data;
		heap_insert(heap, data->id, i);
		array[i] = doclists[i]->head;
	}

}

Document *ite_list(Index *index, Heap *heap,Node **array);
Document *ite_list(Index *index, Heap *heap,Node **array){
	//int doc_id;
	Node *next;
	Document *heap_data, *next_data;
	int list_id;
	//doc_id = (int)heap_peek_key(heap);
	list_id = heap_remove_min(heap);
	//printf("\n%d %d\n",heap->cur_size,heap->max_size );

	heap_data = array[list_id]->data;
	next = array[list_id]->next;
	
	array[list_id] = next;
	

	
    if(next!=NULL){
    	next_data = next->data;
    	heap_insert(heap, next_data->id, list_id);
    	
    }
    //print_heap(heap);
    //printf("%d %d\n",heap->cur_size,heap->max_size );

    return heap_data;


}


// put document id and score in top-k heap 
void put_data_in_heap(Heap *heap, Document data);
void put_data_in_heap(Heap *heap, Document data){
	if (heap->cur_size < heap->max_size){
		heap_insert(heap, data.score, data.id);
	}else {
		if (data.score > heap_peek_key(heap)){
			heap_remove_min(heap);
			heap_insert(heap, data.score, data.id);
		}
	}
}
void print_result_2(Heap *heap);
void print_result_2(Heap *heap){
	int i;
	int size = heap->cur_size;
	Document *list = (Document*)malloc(sizeof(Document)*size);
	for (i=0;i<size;i++){
		list[size-i-1].score = heap_peek_key(heap);
		list[size-i-1].id = heap_remove_min(heap);
	}
	for (i=0;i<size;i++){
		printf("%6d %.6f\n", list[i].id,list[i].score);
	}
	free(list);
	list = NULL;
}


// see Assignment spec: 'Task 2: Priority queue-based multi-way merge approach'
void print_merge_results(Index *index, int n_results) {
	Heap *index_heap;

	Node **node_array = (Node**)malloc(sizeof(*node_array)*index->num_terms);
	
	int *id_list = (int*)malloc(sizeof(int)*index->num_terms);

	
	Heap *top;
	top = new_heap(n_results);
	Document *heap_data;

	Document last_data = {0, 0.0};
	

	//initialise heap
	index_heap = new_heap(index->num_terms); //creat heap
	creat_heap(index, index_heap,node_array);    
	
	
	/*Document *data = next->data;
	printf("next id = %d\n", data->id);*/
	

	while(index_heap->cur_size != 0){

		heap_data = ite_list(index, index_heap, node_array);
		//printf("data  %d\n", heap_data->id);
		if (heap_data->id == last_data.id ){
			//printf("haha\n");
			last_data.score += heap_data->score;

		}else if(heap_data->id != last_data.id ){
			
			if(last_data.score != 0.0){
				put_data_in_heap(top, last_data);
				//printf("%d %f\n",last_data.id,last_data.score);
			}
			last_data = *heap_data;
		}
		//printf("***** %d\n",index_heap->cur_size );
	}
	put_data_in_heap(top, last_data);

	print_result_2(top);
	free_heap(index_heap);
	free_heap(top);
	free(id_list);
	id_list = NULL;


}
