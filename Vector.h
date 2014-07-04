#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdlib.h>
#include <stdio.h>

/* Definindo tipo generico */
template <class Type>

class Vector {
	private:
		Type *elements;
		int size;

    public:
        //construtor
        Vector() {
            this->elements = NULL;
            this->size = 0;
        }

        //destrutor
        ~Vector() {
            free(this->elements);
        }

        /* Pre-Condicao: recebe como parametro um inteiro referente a posicao no vetor
           Pos-Condicao: retorna o elemento do vetor contido na posicao passada por parametro */
        Type get(int pos) {
			if ( (pos >= 0) && (pos < this->size) ) {
				return this->elements[pos];
			}

			return NULL;
		}

        /* Pre-Condicao: nenhuma
           Pos-Condicao: retorna o tamanho do vetor de elementos */
		int getSize() { return this->size; }

        /* Pre-Condicao: recebe como parametro um elemento a ser armazenado no vetor
           Pos-Condicao: adiciona ao vetor o elemento passado como parametro */
        void add(Type elements) {
            this->elements = (Type *) realloc(this->elements, (this->size + 1) * sizeof(Type));
            this->elements[this->size++] = elements;
        }

        /* Pre-Condicao: recebe como parametro um inteiro referente a posicao no vetor
           Pos-Condicao: remove do vetor o elemento da posicao passada */
        void remove(int pos) {
            if ( (pos >= 0) && (pos < this->size) ) {
				this->elements[pos] = this->elements[--this->size];
				this->elements = (Type *) realloc(this->elements,	sizeof(Type) * this->size);
			}
        }
};

#endif
