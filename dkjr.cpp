#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h"

#define VIDE        		0
#define DKJR       		1
#define CROCO       		2
#define CORBEAU     		3
#define CLE 			4

#define AUCUN_EVENEMENT    	0

#define LIBRE_BAS		1
#define LIANE_BAS		2
#define DOUBLE_LIANE_BAS	3
#define LIBRE_HAUT		4
#define LIANE_HAUT		5

void* FctThreadEvenements(void *);
void* FctThreadCle(void *);
void* FctThreadDK(void *);
void* FctThreadDKJr(void *);
void* FctThreadScore(void *);
void* FctThreadEnnemis(void *);
void* FctThreadCorbeau(void *);
void* FctThreadCroco(void *);

void initGrilleJeu();
void setGrilleJeu(int l, int c, int type = VIDE, pthread_t tid = 0);
void afficherGrilleJeu();

void HandlerSIGUSR1(int);
void HandlerSIGUSR2(int);
void HandlerSIGALRM(int);
void HandlerSIGINT(int);
void HandlerSIGQUIT(int);
void HandlerSIGCHLD(int);
void HandlerSIGHUP(int);

void DestructeurVS(void *p);

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;

pthread_key_t keySpec;

bool MAJDK = false;
int  score = 0;
bool MAJScore = true;
int  delaiEnnemis = 4000;
int  positionDKJr = 1;
int  evenement = AUCUN_EVENEMENT;
int etatDKJr;

typedef struct
{
  int type;
  pthread_t tid;
} S_CASE;

S_CASE grilleJeu[4][8];

typedef struct
{
  bool haut;
  int position;
} S_CROCO;

// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	
	printf("Thread principal : debut\n");
	fflush(stdout);

	ouvrirFenetreGraphique();

	pthread_mutex_init(&mutexEvenement,NULL);
  pthread_mutex_init(&mutexGrilleJeu, NULL);
  setGrilleJeu(0, 1, VIDE, 0); // Initialisation de la grilleJeu

  pthread_create(&threadDKJr, NULL , &FctThreadDKJr,NULL);
  pthread_create(&threadEvenements,NULL,&FctThreadEvenements,NULL);
  pthread_create(&threadCle, NULL, &FctThreadCle, NULL);
  
  

  pause();
	

  printf("thread principal : fin\n");
  return 0;
}

// -------------------------------------

void initGrilleJeu()
{
  int i, j;   
  
  pthread_mutex_lock(&mutexGrilleJeu);

  for(i = 0; i < 4; i++)
    for(j = 0; j < 7; j++)
      setGrilleJeu(i, j);

  pthread_mutex_unlock(&mutexGrilleJeu);
}

// -------------------------------------

void setGrilleJeu(int l, int c, int type, pthread_t tid)
{
  grilleJeu[l][c].type = type;
  grilleJeu[l][c].tid = tid;
}

// -------------------------------------

void afficherGrilleJeu()
{   
   for(int j = 0; j < 4; j++)
   {
       for(int k = 0; k < 8; k++)
          printf("%d  ", grilleJeu[j][k].type);
       printf("\n");
   }

   printf("\n");   
}

void* FctThreadCle(void*)
{
	int i=0;
	int position= 0;
  int direction = 1; // 1: descend, -1: monte
  while(1) 
  {
  	while(i<=3)
  	{
	    if (position == 0) 
	    {
	      direction = 1;
	      pthread_mutex_lock(&mutexGrilleJeu); 
	      setGrilleJeu(3, 14,VIDE);
	      afficherCle(4);
	      pthread_mutex_unlock(&mutexGrilleJeu);
	     	struct timespec temps = { 0, 700000000 };
				nanosleep(&temps, NULL);
				effacerCarres(3,14,1,1);
				effacerCarres(3,13,1,1);
				effacerCarres(4,14,1,1);
				i++;
	    } 
	    else if (position == 3) 
	    {
	      direction = -1;
	      pthread_mutex_lock(&mutexGrilleJeu);
	      setGrilleJeu(4, 12, CLE);
	      afficherCle(1);
	      pthread_mutex_unlock(&mutexGrilleJeu);
	      struct timespec temps = { 0, 700000000 };
				nanosleep(&temps, NULL);
				effacerCarres(3,12,1,1);
				effacerCarres(4,12,1,1);
				i++;	
	    }
	    else if(position == 1)
	    {
	    	direction = 1;
	    	pthread_mutex_lock(&mutexGrilleJeu);
	      setGrilleJeu(4, 13, VIDE);
	      afficherCle(3);
	      pthread_mutex_unlock(&mutexGrilleJeu);
	    	struct timespec temps = { 0, 700000000 };
				nanosleep(&temps, NULL);
				effacerCarres(3,13,1,1);
				effacerCarres(4,14,1,1);
				effacerCarres(4,13,1,1);
				i++;
	    }
	    else if(position == 2)
	    {
	    	direction = 1;
	    	pthread_mutex_lock(&mutexGrilleJeu);
	      setGrilleJeu(4, 13, VIDE);
	      afficherCle(2);
	      pthread_mutex_unlock(&mutexGrilleJeu);
	    	struct timespec temps = { 0, 700000000 };
				nanosleep(&temps, NULL);
				effacerCarres(3,13,1,1);
				effacerCarres(4,13,1,1);
				i++;
	    }
    position += direction;
  }
  i=3;

  while(i!=0)
  {
  	if (position == 0) 
	    {
	      direction=
	      pthread_mutex_lock(&mutexGrilleJeu); 
	      setGrilleJeu(3, 14,VIDE);
	      afficherCle(4);
	      pthread_mutex_unlock(&mutexGrilleJeu);
				effacerCarres(3,14,1,1);
				effacerCarres(3,13,1,1);
				effacerCarres(4,14,1,1);
				i--;
	    } 
	    else if (position == 3) 
	    {
	      direction = -1;
	      pthread_mutex_lock(&mutexGrilleJeu);
	      setGrilleJeu(4, 12, CLE);
	      afficherCle(1);
	      pthread_mutex_unlock(&mutexGrilleJeu);
	      struct timespec temps = { 0, 700000000 };
				nanosleep(&temps, NULL);
				effacerCarres(3,12,1,1);
				effacerCarres(4,12,1,1);
				i--;	
	    }
	    else if(position == 1)
	    {
	    	direction = -1;
	    	pthread_mutex_lock(&mutexGrilleJeu);
	      setGrilleJeu(4, 13, VIDE);
	      afficherCle(3);
	      pthread_mutex_unlock(&mutexGrilleJeu);
	    	struct timespec temps = { 0, 700000000 };
				nanosleep(&temps, NULL);
				effacerCarres(3,13,1,1);
				effacerCarres(4,14,1,1);
				effacerCarres(4,13,1,1);
				i--;
	    }
	    else if(position == 2)
	    {
	    	direction = -1;
	    	pthread_mutex_lock(&mutexGrilleJeu);
	      setGrilleJeu(4, 13, VIDE);
	      afficherCle(2);
	      pthread_mutex_unlock(&mutexGrilleJeu);
	    	struct timespec temps = { 0, 700000000 };
				nanosleep(&temps, NULL);
				effacerCarres(3,13,1,1);
				effacerCarres(4,13,1,1);
				i--;
	    }
    position += direction;
  }
  i=0;
 }
}

void* FctThreadEvenements(void*)
{
	int evt;
	struct timespec temps1 = {0,100000000};
	nanosleep(&temps1,NULL);

	while (1)
	{
	    evt = lireEvenement();
	    pthread_mutex_lock(&mutexEvenement);

	    switch (evt)
	    {
				case SDL_QUIT:
					exit(0);
				case SDLK_UP:
					printf("KEY_UP\n");
					evenement = evt;
					break;
				case SDLK_DOWN:
					printf("KEY_DOWN\n");
					evenement = evt;
					break;
				case SDLK_LEFT:
					printf("KEY_LEFT\n");
					evenement = evt;
					break;
				case SDLK_RIGHT:
					printf("KEY_RIGHT\n");
					evenement = evt;
					break;
	    }
	    pthread_mutex_unlock(&mutexEvenement);
	    pthread_kill(threadDKJr,l);
	    nanosleep(&temps1,NULL);


	    pthread_mutex_lock(&mutexEvenement);
	    evenement = AUCUN_EVENEMENT;
	    pthread_mutex_unlock(&mutexEvenement);
	}

}

void* FctThreadDKJr(void* p)
{

	int evt;
	pthread_mutex_lock(&mutexGrilleJeu);

	setGrilleJeu(3, 1, DKJR); 
 	afficherDKJr(11, 9, 1);
 	etatDKJr = LIBRE_BAS;
 	positionDKJr = 1;

 	pthread_mutex_unlock(&mutexGrilleJeu);
 	while(1)
 	{
 		pause();
 		evt = lireEvenement();
 		pthread_mutex_lock(&mutexEvenement);
 		pthread_mutex_lock(&mutexGrilleJeu);

 		switch(etatDKJr)
 		{
 			case LIBRE_BAS :
 				switch(evt)
 				{
 					case SDLK_LEFT:

 					if (positionDKJr > 1)
				 {
					 setGrilleJeu(3, positionDKJr);
					 effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					 positionDKJr--;
					 setGrilleJeu(3, positionDKJr, DKJR);
					 afficherDKJr(11, (positionDKJr * 2) + 7, 
					 ((positionDKJr - 1) % 4) + 1);
					 //positionDKJr -= 2;
				 }
				 	evenement = evt;
				 break;

				 case SDLK_RIGHT:

				 if(positionDKJr < 7 )
				 {
				 	setGrilleJeu(3,positionDKJr);
				 	effacerCarres(11,(positionDKJr*2) + 7,2,2);
				 	positionDKJr++;
				 	setGrilleJeu(3,positionDKJr,DKJR);
				 	afficherDKJr(11,(positionDKJr*2)+7,
				 	((positionDKJr-1)%4)+1);
				 }
				 	evenement = evt;
				 break;
				 /*case SDLK_UP:

				 if(positionDKJr >)*/


 				}
 		}
 		pthread_mutex_unlock(&mutexGrilleJeu);
 		pthread_mutex_unlock(&mutexEvenement);
 	}

}

void HandlerSIGQUIT(int sig)
{
	printf("\nHandler signal : SIGQUIT");
	//exit(0);
}
