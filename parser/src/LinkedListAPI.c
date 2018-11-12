#include "LinkedListAPI.h"
#include "assert.h"


List * initializeList(char* (*printFunction)(void* toBePrinted),void (*deleteFunction)(void* toBeDeleted),int (*compareFunction)(const void* first,const void* second)){
    //Asserts create a partial function...
    assert(printFunction != NULL);
    assert(deleteFunction != NULL);
    assert(compareFunction != NULL);

    List * tmpList = malloc(sizeof(List));

	tmpList->head = NULL;
	tmpList->tail = NULL;

	tmpList->length = 0;

	tmpList->deleteData = deleteFunction;
	tmpList->compare = compareFunction;
	tmpList->printData = printFunction;

	return tmpList;
}

void freeList(List* list){
    clearList(list);
	free(list);
}

Node* initializeNode(void* data){
	Node* tmpNode = (Node*)malloc(sizeof(Node));

	if (tmpNode == NULL){
		return NULL;
	}

	tmpNode->data = data;
	tmpNode->previous = NULL;
	tmpNode->next = NULL;

	return tmpNode;
}

void insertBack(List* list, void* toBeAdded){
	if (list == NULL || toBeAdded == NULL){
		return;
	}

	(list->length)++;

	Node* newNode = initializeNode(toBeAdded);

    if (list->head == NULL && list->tail == NULL){
        list->head = newNode;
        list->tail = list->head;
    }else{
		newNode->previous = list->tail;
        list->tail->next = newNode;
    	list->tail = newNode;
    }
}

void insertFront(List* list, void* toBeAdded){
	if (list == NULL || toBeAdded == NULL){
		return;
	}

	(list->length)++;

	Node* newNode = initializeNode(toBeAdded);

    if (list->head == NULL && list->tail == NULL){
        list->head = newNode;
        list->tail = list->head;
    }else{
		newNode->next = list->head;
        list->head->previous = newNode;
    	list->head = newNode;
    }
}

void* getFromFront(List * list){
	if (list->head == NULL){
		return NULL;
	}

	return list->head->data;
}

void* getFromBack(List * list){
	if (list->tail == NULL){
		return NULL;
	}

	return list->tail->data;
}

void* deleteDataFromList(List* list, void* toBeDeleted){
	if (list == NULL || toBeDeleted == NULL){
		return NULL;
	}

	Node* tmp = list->head;

	while(tmp != NULL){
		if (list->compare(toBeDeleted, tmp->data) == 0){
			//Unlink the node
			Node* delNode = tmp;

			if (tmp->previous != NULL){
				tmp->previous->next = delNode->next;
			}else{
				list->head = delNode->next;
			}

			if (tmp->next != NULL){
				tmp->next->previous = delNode->previous;
			}else{
				list->tail = delNode->previous;
			}

			void* data = delNode->data;
			free(delNode);

			(list->length)--;

			return data;

		}else{
			tmp = tmp->next;
		}
	}

	return NULL;
}

void insertSorted(List *list, void *toBeAdded){
	if (list == NULL || toBeAdded == NULL){
		return;
	}

	(list->length)++;

	if (list->head == NULL){
		insertBack(list, toBeAdded);
		return;
	}

	if (list->compare(toBeAdded, list->head->data) <= 0){
		insertFront(list, toBeAdded);
		return;
	}

	if (list->compare(toBeAdded, list->tail->data) > 0){
		insertBack(list, toBeAdded);
		return;
	}

	Node* currNode = list->head;

	while (currNode != NULL){
		if (list->compare(toBeAdded, currNode->data) <= 0){

			char* currDescr = list->printData(currNode->data);
			char* newDescr = list->printData(toBeAdded);

			//printf("Inserting %s before %s\n", newDescr, currDescr);

			free(currDescr);
			free(newDescr);

			Node* newNode = initializeNode(toBeAdded);
			newNode->next = currNode;
			newNode->previous = currNode->previous;
			currNode->previous->next = newNode;
			currNode->previous = newNode;

			return;
		}

		currNode = currNode->next;
	}

	return;
}

char* toString(List * list){
	ListIterator iter = createIterator(list);
	char* str;
	int temp = false;

	str = (char*)malloc(sizeof(char));
	strcpy(str, "");

	void* elem;
	while((elem = nextElement(&iter)) != NULL){
		char* currDescr = list->printData(elem);
		int newLen = strlen(str)+50+strlen(currDescr);
		str = (char*)realloc(str, newLen);
		//===> Removing the newline char because it's messing with my string structure for printing the card
		//strcat(str, "\n");
		if (temp == true) {
			strcat(str, ";");
		}
		strcat(str, currDescr);
		temp = true;
		free(currDescr);
	}

	return str;
}

int getLength(List* list){
	return list->length;
}

void* findElement(List * list, bool (*customCompare)(const void* first,const void* second), const void* searchRecord){
	if (customCompare == NULL)
		return NULL;

	ListIterator itr = createIterator(list);

	void* data = nextElement(&itr);
	while (data != NULL)
	{
		if (customCompare(data, searchRecord))
			return data;

		data = nextElement(&itr);
	}

	return NULL;
}

void clearList(List* list) {
    if (list == NULL){
  		return;
  	}
  	if (list->head == NULL && list->tail == NULL){
  		return;
  	}
  	Node* tmp;
  	while (list->head != NULL){
  		list->deleteData(list->head->data);
  		tmp = list->head;
  		list->head = list->head->next;
  		list->length--;
  		free(tmp);
  	}
  	list->head = NULL;
  	list->tail = NULL;
}

ListIterator createIterator(List * list){
    ListIterator mover;
    mover.current = list->head;
    return mover;
}

void* nextElement(ListIterator* iter){
    Node* temp = iter->current;
    if (temp != NULL){
        iter->current = iter->current->next;
        return temp->data;
    }else{
        return NULL;
    }
}
