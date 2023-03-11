#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int *vet, **m, max, *valores;
FILE *fp;

/* struct que vai conter o id da thread e o numero da cidade que a thread vai usar*/
typedef struct
{
    int id;
    int valor;
    int multi;
}argumentos;

/*verifica se a cidade está na lista de visitados*/
int verifica(int a,int v[], int m){
    int i;
    for (i = 0; i < m; i++)
    {
        if (v[i] == a)
        {
            /* printf("existe %d \n", a);*/
            return 1;
        }
        
    }
    /* printf("não existe %d \n", a);*/
    return 0;

}

/*adiciona a cidade a lista de visitados*/
void adiciona(int a,int v[], int m){
    int i;
    for (i = 0; i < m; i++)
    {
        if (v[i] == -1)
        {
            v[i] = a;
            /* printf("adicionado %d \n", a);*/
            break;
        }
        
    }
}

/* inicializa o vetor de visitado com -1 para representar vazio*/
void visitados(int v[]){
    int i;
    for ( i = 1; i < max; i++)
    {
        v[i] = -1;
        /*printf("ação visitados\n");*/
    }
}

/* adiciona os numeros das cidades em um vetor para ser passado para uma thread*/
void *t_valores(void *arg){
    int i;
    for ( i = 0; i < max; i++)
    {
        valores[i] = i;
        /* printf("valores\n");*/
    }
    return NULL;
}

/* thread que lê a matriz do arquico tsp*/
void *t_matriz(void *arg){
    int i, j , b;
    for ( i = 0; i < max; i++)
    {
        for ( j = 0; j < max; j++)
        {
            fscanf(fp, "%d", &b);
            m[i][j] = b;
        }
        /* printf("leitura\n");*/
    }
    return NULL;
}
void *t_caixeiro(void *arg){
    int k, l;
    argumentos *args = (argumentos *)arg;
    int porthread = args->valor;
    int id = args->id;
    int multi = args->multi;
    /* cada thread vai rodar com um intervalo de cidades/ valor inicial de k é o numero da primeira cidade*/
    for (k = multi*porthread, l = 0; l < porthread; k++, l++)
    {
        int i, j, prox, menor, total = 0, atual, inicio, *v;
        inicio = k;
        atual = k;
        
        v = malloc(max * sizeof (int*));
        visitados(v);
        v[0] = inicio;
        for (j = 0; j < max-1; j++)
        {
            /* valor alto para que o primeiro seja menor inicialmente*/
            menor = 1000;
            for ( i = 0; i < max; i++)
            {
                /*verifica se a distacia a cidade até o vizinho seja menor que o menor determinado anteriormente e se é uma cidade diferente da atual*/
                if (m[atual][i] < menor && i != atual)
                {
                    /*verifica se ja foi visitado*/
                    if (verifica(i, v, max) == 0)
                    {
                        /*determina qual o menor, que sera o proximo destino*/
                        menor = m[atual][i];
                        prox = i;
                    }
                }
            }
            /*o menor é adicionado na lista de visitador e sera o proximo destino*/
            adiciona(prox, v, max);
            /*adiciona a distancia no total percorrido*/
            total += menor;
            atual = prox;
        }
    
    
    
    /*adiciona a distancia da ultima cidade visitada até a cidade inicial*/
    total+=m[atual][inicio];
    /*exibe total percorrido*/
    printf("thread(%d)inicial(%d)total: %d\n", id, inicio, total);
    vet[inicio] = total;
    free(v);
    }

}




int main(int argc, char const *argv[])
{
    pthread_t thread[3];
    
    argumentos *arg;
    int i, j, atual = 3, total = 0, k = 0, num, porthread, melhor, melhortotal;
    
    /* arquivo aberto*/
    fp = fopen("teste1.tsp", "r");


    /*le o numero de cidades do arquivo tsp e lê em quantas threads vão ser utilizadas a partir do teclado*/
    fscanf(fp, "%d", &max);
    printf("numero de cidades: %d\n", max);

    /* impede que tenha mais threads do que cidades*/
    do
    {
        printf("quantas threads utilizar? ");
        scanf("%d", &num);
    } while (num > max || num < 0);

    /*aloca a memoria da matriz e do vetor de visitados*/
    m = malloc(max * sizeof (int*));
    arg = malloc(max * sizeof (argumentos));
    vet = malloc(max * sizeof (int*));
    valores = malloc(max * sizeof (int*));
    pthread_t *funcao = (pthread_t *) malloc(num * sizeof(pthread_t));

    for ( i = 0; i < max; i++)
    {
        m[i] = malloc(max * sizeof (int));
    }
 
    /*recolhe as informações da matriz do arquivo tsp*/
    pthread_create(&thread[0], NULL, t_matriz, NULL);

    /*preenche um vetor com os numeros das cidades para que possam ser passadas para a thread t_caixeiro*/
    pthread_create(&thread[1], NULL, t_valores, NULL);

    printf("threads criadas\n");

    /*espera que as threads terminem*/
    pthread_join(thread[0], NULL);
    printf("primeira thread finalizada\n");
    pthread_join(thread[1], NULL);
    printf("segunda thread finalizada\n");

    /*separa as cidades entre o numero de threads definido pelo usuario*/
    porthread = max/num;
    /*divisão padrão*/
    for ( i = 0; i < num; i++)
    {
        arg[i].id= i;
        arg[i].multi = i;
        arg[i].valor = porthread; 
        pthread_create(&funcao[i], NULL, t_caixeiro, &arg[i]);
    }
    /* se houver resto na divisão, será dividido entre as primeiras threads*/
    for ( i = 0; i < num; i++)
    {
        pthread_join(funcao[i], NULL);
    }

    if(max%num > 0){
        printf("sobrou %d\n", max%num);
        for (j=0, i = max-(max%num); j < max%num; i++, j++)
        {
            arg[i].multi = i;
            arg[i].id = j;
            arg[i].valor = 1;
            pthread_create(&funcao[j], NULL, t_caixeiro, &arg[i]);
            
        }
        /* aguarda que todas as threads acabem*/
        for ( i = 0; i < max%num; i++)
        {
            pthread_join(funcao[i], NULL);
        }
    }

    
    /* exibe o total de todas as cidades e define qual cidade inicial possui o menor caminho para visitar todas as outras*/
    melhor = 0;
    melhortotal = vet[0];
    for (i = 0; i < max; i++)
    {
        /*printf("cidade %d: %d ", i, vet[i]);*/
        if (melhortotal > vet[i])
        {
            melhor = i;
            melhortotal = vet[i];
        }
    }
    printf("\n\n%d é a melhor cidade inicial, sendo %d a distancia percorrida para visitar todas as cidades\n\n", melhor, melhortotal);

    /*libera memoria alocada*/
    for ( i = 0; i < max; i++)
    {
        free(m[i]);
    }
    free(m);
    free(vet);
    free(arg);
    free(valores);

    return 0;
}
