//my402list.c
#include "my402list.h"
#include <stdlib.h>
#include <stdio.h>

int  My402ListLength(My402List* list){
	return list->num_members;
}
int  My402ListEmpty(My402List* list){
	return list->num_members <= 0;
}

int  My402ListAppend(My402List* list, void* newObj){
	My402ListInsertBefore(list,newObj,&(list->anchor));
	return 0;
}
int  My402ListPrepend(My402List* list, void* newObj){
	My402ListInsertAfter(list,newObj,&(list->anchor));
	return 0;
}
void My402ListUnlink(My402List* list, My402ListElem* elem){
	if(My402ListEmpty(list)){
		return;
	}
	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
	free(elem);
	list->num_members--;
}
void My402ListUnlinkAll(My402List* list){
	My402ListElem *elem=NULL;
	for (elem=My402ListFirst(list);  elem != NULL;  elem=My402ListNext(list, elem)){
        if(elem == My402ListFirst(list)){
			continue;
		}else{
			My402ListUnlink(list,My402ListPrev(list,elem));
		}
		//Foo *foo=(Foo*)(elem->obj);
    }
}


int  My402ListInsertAfter(My402List* list, void* newObj, My402ListElem* elem){
	My402ListElem *newElem = malloc(sizeof(My402ListElem));
	newElem->obj = newObj;
	newElem->next = elem->next;
	newElem->prev = elem;
	elem->next->prev = newElem;
	elem->next = newElem;
	list->num_members++;
	return 0;
}


int  My402ListInsertBefore(My402List* list, void* newObj, My402ListElem* elem){
	My402ListElem *newElem = malloc(sizeof(My402ListElem));
	newElem->obj = newObj;
	newElem->next = elem;
	newElem->prev = elem->prev;
	elem->prev->next = newElem;	
	elem->prev = newElem;
	list->num_members++;
	return 0;
}


My402ListElem *My402ListFirst(My402List* list){
	if(My402ListEmpty(list)){
		return NULL;
	}
	return list->anchor.next;
}

My402ListElem *My402ListLast(My402List* list){
	if(My402ListEmpty(list)){
		return NULL;
	}
	return list->anchor.prev;
}

My402ListElem *My402ListNext(My402List* list, My402ListElem* elem){
	if(elem->next == &(list->anchor)){
		return NULL;
	}
	return elem->next;
}

My402ListElem *My402ListPrev(My402List* list, My402ListElem* elem){
	if(elem->prev == &(list->anchor)){
		return NULL;
	}
	return elem->prev;
}

My402ListElem *My402ListFind(My402List* list, void* newObj){
	My402ListElem *elem=NULL;

    for (elem=My402ListFirst(list);
        elem != NULL;
        elem=My402ListNext(list, elem)) {
        if(elem->obj == newObj){
			return elem;
		}
		//Foo *foo=(Foo*)(elem->obj);
    }
	return NULL;
}

int My402ListInit(My402List* list){
	list->num_members = 0;
	list->anchor.next = &(list->anchor);
	list->anchor.prev = &(list->anchor);
	list->anchor.obj = NULL;
	return 0;
}
