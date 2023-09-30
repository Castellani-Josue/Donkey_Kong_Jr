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
pthread_t threadCorbeau;
pthread_t threadCroco;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;
pthread_mutex_t mutexDelaiEnemi;

pthread_key_t keySpec;

bool MAJDK = false;
int vie;
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
struct sigaction A;
sigset_t mask;
// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
		
	 	
    A.sa_handler = HandlerSIGQUIT;
    A.sa_flags = 0;

    if (sigaction(SIGQUIT,&A,NULL) == -1)
    {
        perror("erreur de sigaction\n");
        exit(1);
    }
    sigemptyset(&mask);
		sigaddset(&mask, SIGQUIT); 
		sigprocmask(SIG_BLOCK, &mask, NULL);

	//signal(SIGQUIT, HandlerSIGQUIT);

		//Armement SIGUSR1
	sigemptyset(&A.sa_mask);
	A.sa_handler = HandlerSIGUSR1;
	A.sa_flags = 0;
	sigaction(SIGUSR1, &A, NULL);

	//Armement SIGUSR2
	sigemptyset(&A.sa_mask);
	A.sa_handler = HandlerSIGUSR2;
	A.sa_flags = 0;
	sigaction(SIGUSR2, &A, NULL);

	//Armerment SIGALARM
	sigemptyset(&A.sa_mask);
	A.sa_handler = HandlerSIGALRM;
	A.sa_flags = 0;
	sigaction(SIGALRM, &A, NULL);

	//Armement SIGINT
	sigemptyset(&A.sa_mask);
	A.sa_handler = HandlerSIGINT;
	A.sa_flags = 0;
	sigaction(SIGINT, &A, NULL);

	//Armement SIGHUP
	sigemptyset(&A.sa_mask);
	A.sa_handler = HandlerSIGHUP;
	A.sa_flags = 0;
	sigaction(SIGHUP, &A, NULL);

	//Armement SIGCHLD
	sigemptyset(&A.sa_mask);
	A.sa_handler = HandlerSIGCHLD;
	A.sa_flags = 0;
	sigaction(SIGCHLD, &A, NULL);

	//sigprocmask(SIG_BLOCK, &mask, NULL); bonne place
	
	printf("Thread principal : debut\n");
	fflush(stdout);

	ouvrirFenetreGraphique();

	pthread_mutex_init(&mutexEvenement,NULL);
  pthread_mutex_init(&mutexGrilleJeu, NULL);
  pthread_mutex_init(&mutexDK,NULL);
  pthread_mutex_init(&mutexScore,NULL);
  pthread_mutex_init(&mutexDelaiEnemi,NULL);
  pthread_cond_init(&condDK,NULL);
  pthread_cond_init(&condScore,NULL);

  setGrilleJeu(0, 1, VIDE, 0); // Initialisation de la grilleJeu
  

  pthread_create(&threadEvenements,NULL,FctThreadEvenements,NULL);
  pthread_create(&threadCle, NULL, FctThreadCle, NULL);
  pthread_create(&threadDK,NULL,FctThreadDK,NULL);
  pthread_create(&threadScore,NULL,FctThreadScore,NULL);
  pthread_create(&threadEnnemis,NULL,&FctThreadEnnemis,NULL);

  //assignation destructeur clé
  pthread_key_create(&keySpec, DestructeurVS);


  vie = 0;
  while(vie<3)
  {

  		pthread_create(&threadDKJr, NULL , FctThreadDKJr,NULL);
  	  pthread_join(threadDKJr,NULL);
  	  vie++;
  	  afficherEchec(vie);
  }

  sleep(3);
  


  printf("thread principal : fin\n");
  exit(0);
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
    int position = 1;
    // int position=0;
  	int direction = 1; // 1: descend, -1: monte
  while(1)
  {

      pthread_mutex_lock(&mutexGrilleJeu);
      if(position == 1)
      {
          setGrilleJeu(0, 1, CLE);
      }
      else
      {
          setGrilleJeu(0, 1, VIDE); 
      }
      afficherCle(position);
      pthread_mutex_unlock(&mutexGrilleJeu);
      struct timespec temps = { 0, 700000000 };
        nanosleep(&temps, NULL);

        if(position == 1)
        {
            effacerCarres(3,12,2,1);
        }
        else if(position == 2)
        {
            effacerCarres(3,13,2,1);
        }
        else if(position == 3)
        {
            effacerCarres(3,13,2,2);
        }
        else if(position == 4)
        {
            effacerCarres(3,13,2,2);
        }
      i++;
      if(direction > 0) //suivi de la position qui vas de 1 a 4 et de 4 a 1
      {
          position++;
      }
      else
      {
          position--;
      }
      if(i%3 == 0)
      {
          direction = -direction; 
      }
  }
}

void* FctThreadEvenements(void*)
{
	int evt;
	struct timespec temps1 = {0,100000000};
	

	while (1)
	{
	    evt = lireEvenement();
	    pthread_mutex_lock(&mutexEvenement);

	    switch (evt)
	    {
				case SDL_QUIT:
					exit(0);
					break;
				case SDLK_UP:
					printf("KEY_UP\n");				
					evenement = evt;
					break;
				case SDLK_DOWN:
					printf("\nKEY_DOWN\n");
					evenement = evt;
					break;
				case SDLK_LEFT:
					printf("\nKEY_LEFT\n");
					evenement = evt;
					break;
				case SDLK_RIGHT:
					printf("\nKEY_RIGHT\n");
					evenement = evt;
					break;
	    }
	    pthread_mutex_unlock(&mutexEvenement);
	    pthread_kill(threadDKJr,SIGQUIT);
	    nanosleep(&temps1,NULL);


	    pthread_mutex_lock(&mutexEvenement);
	    evenement = AUCUN_EVENEMENT;
	    pthread_mutex_unlock(&mutexEvenement);
	    
	}
 
}

void* FctThreadDKJr(void* p)
{
	struct timespec temps2 = {0,200000000};
	struct timespec temps = {0,100000000};
	struct timespec temp3 = {0,200000000};
	struct timespec temps4 = { 1, 400000000 };

 	bool on = true, point = false;
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	for(int i = 1; i<5 ; i++)
	{
		if(grilleJeu[3][i].type == CROCO)
			pthread_kill(grilleJeu[3][i].tid, SIGUSR2);
	}
	for(int i = 0; i<4 ; i++)
	{
		if(grilleJeu[2][i].type == CORBEAU)
			pthread_kill(grilleJeu[2][i].tid, SIGUSR1);
	}

	pthread_mutex_lock(&mutexGrilleJeu);

	nanosleep(&temp3,NULL);
	effacerCarres(11,9,2,2);
	effacerCarres(11,7,2,2);
	setGrilleJeu(3, 1, DKJR);
	afficherDKJr(11, 9, 1);
 	etatDKJr = LIBRE_BAS;
 	positionDKJr = 1;

 	pthread_mutex_unlock(&mutexGrilleJeu);

 	

 	while(on)
 	{
 		pause();
 		pthread_mutex_lock(&mutexEvenement);
 		pthread_mutex_lock(&mutexGrilleJeu);
 		switch(etatDKJr)
 		{
 			case LIBRE_BAS :
 				switch(evenement)
 				{
 					case SDLK_LEFT:

		 				 if (positionDKJr >= 2)
						 {
							 	if(grilleJeu[3][(positionDKJr)-1].type == CROCO)
							 	{
							 		printf("1\n");
							 		effacerCarres(11,(positionDKJr*2)+ 7,2,2);
							 		setGrilleJeu(3, positionDKJr); 
							 		pthread_kill(grilleJeu[3][(positionDKJr)-1].tid, SIGUSR2);
							 		on = false;
							 	}
							 	else
							 	{
							 		setGrilleJeu(3, positionDKJr);
								  effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
								  positionDKJr--;
								  setGrilleJeu(3, positionDKJr, DKJR);
								  afficherDKJr(11, (positionDKJr * 2) + 7, 
								  ((positionDKJr - 1) % 4) + 1);
								  //positionDKJr -= 2;
							 	}
						 }
				 
				 break;

				 case SDLK_RIGHT:

				 if(positionDKJr < 7 )
				 {
				 	if(grilleJeu[3][(positionDKJr)+1].type == CROCO)
					{
						printf("2\n");
						effacerCarres(11,(positionDKJr*2)+ 7,2,2);
						setGrilleJeu(3, positionDKJr);
					 	pthread_kill(grilleJeu[3][(positionDKJr)+1].tid, SIGUSR2);
					 	on = false;
					}
					else
					{
						setGrilleJeu(3,positionDKJr);
				 		effacerCarres(11,(positionDKJr*2) + 7,2,2);
				 		positionDKJr++;
				 		setGrilleJeu(3,positionDKJr,DKJR);
				 		afficherDKJr(11,(positionDKJr*2)+7,
				 		((positionDKJr-1)%4)+1);
					}
				 }

				 break;
				 case SDLK_UP:
				 	if(grilleJeu[2][(positionDKJr)].type == CORBEAU)
					{
						printf("3\n");
						effacerCarres(10,(positionDKJr*2)+ 7,2,2);
				 		effacerCarres(11,(positionDKJr*2)+ 7,2,2);
				 		setGrilleJeu(3, positionDKJr, 0);  
						pthread_kill(grilleJeu[2][(positionDKJr)].tid, SIGUSR1);
						on = false;
					}
					else
					{
						setGrilleJeu(3, positionDKJr, 0);//enleve dk
						if(positionDKJr == 2 || positionDKJr == 3 || positionDKJr == 4 || positionDKJr == 6)
						 {

						 	if(grilleJeu[3][(positionDKJr)+1].type == CROCO)
						 	{ //pour les point
						 		point = true;
						 	}
						 	effacerCarres(11,(positionDKJr*2)+ 7,2,2);
						 	setGrilleJeu(2,positionDKJr,DKJR);
						 	afficherDKJr(10,(positionDKJr*2)+7,
						 	8);
						 	pthread_mutex_unlock(&mutexGrilleJeu); 
						 	nanosleep(&temps4, NULL);

						 	if(grilleJeu[3][(positionDKJr)].type == CROCO) //test si croco en dessous
						 	{
						 		printf("4\n");
						 		effacerCarres(10,(positionDKJr*2)+ 7,2,2);
						 		effacerCarres(11,(positionDKJr*2)+ 7,2,2);
						 		setGrilleJeu(3, positionDKJr);
						 		pthread_kill(grilleJeu[3][(positionDKJr)].tid, SIGUSR2);
						 		on = false;
						 	}
						 	else
						 	{
						 		pthread_mutex_lock(&mutexGrilleJeu);
					 	 		setGrilleJeu(2, positionDKJr);//enleve dk
								effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
								setGrilleJeu(3, positionDKJr, 1);
								effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
								afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
								if(point == true)
								{
									pthread_mutex_lock(&mutexScore);
				 					score = score + 1;
				 					MAJScore = true;
				 					pthread_mutex_unlock(&mutexScore);
				 					pthread_cond_signal(&condScore);
									point = false;
								}
						 	}


						 }
						 if(positionDKJr == 1 || positionDKJr == 5)
						 {
						 	
						 		setGrilleJeu(3,positionDKJr);
						 	 	effacerCarres(11,(positionDKJr*2)+ 7,2,2);		
						 		setGrilleJeu(2,positionDKJr,DKJR);
						 		afficherDKJr(10,(positionDKJr*2)+7,7);
						 		etatDKJr=LIANE_BAS;
						 }

						 if(positionDKJr == 7)
			 				{
			 					setGrilleJeu(3,positionDKJr);
						 	 	effacerCarres(11,(positionDKJr*2)+ 7,2,2);
						 		setGrilleJeu(2,positionDKJr,DKJR);
						 		afficherDKJr(10,(positionDKJr*2)+7,5);
						 		etatDKJr=DOUBLE_LIANE_BAS;

			 				}	
					}				

				 break;

 				}
 			case LIANE_BAS  : 

 				switch(evenement)
 				{
 					case SDLK_DOWN :

 					if(positionDKJr == 1 || positionDKJr == 5)
 					{
 						if(grilleJeu[3][(positionDKJr)].type == CROCO)
 						{
 							printf("5\n");
						 	effacerCarres(10,(positionDKJr*2)+ 7,2,2);
						 	effacerCarres(11,(positionDKJr*2)+ 7,2,2);
						 	setGrilleJeu(2, positionDKJr);
						 	pthread_kill(grilleJeu[3][(positionDKJr)].tid, SIGUSR2);
						 	on = false;
 						}
 						else
 						{
	 						setGrilleJeu(2,positionDKJr);
					 	 	effacerCarres(11,(positionDKJr*2)+ 7,2,2);
					 	 	effacerCarres(10,17,2,2);
					 	 	effacerCarres(10,9,2,2);		
					 		setGrilleJeu(3,positionDKJr,DKJR);
					 		afficherDKJr(11,(positionDKJr*2)+7,
					 		((positionDKJr-1)%4)+1);
					 		etatDKJr=LIBRE_BAS;
 						}

 					}
 					// effacerCarres();
 					break;
 				}
 			break;

 			case DOUBLE_LIANE_BAS :

 			switch(evenement)
 			{
 				case SDLK_UP : 

 				if(positionDKJr == 7)
 				{

 					setGrilleJeu(2,positionDKJr);
			 	 	effacerCarres(11,21,2,2);
			 	 	effacerCarres(10,21,2,2);
			 		setGrilleJeu(1,positionDKJr,DKJR);
			 		afficherDKJr(11,(positionDKJr*2)+7,6);
			 		etatDKJr=LIBRE_HAUT;

			 	}
	 				
	 				break;

	 				case SDLK_DOWN :

 					if(positionDKJr == 7)
 					{
 						if(grilleJeu[3][(positionDKJr)].type == CROCO)
 						{
 							printf("6\n");
						 	effacerCarres(10,(positionDKJr*2)+ 7,2,2);
						 	setGrilleJeu(2, positionDKJr);
						 	pthread_kill(grilleJeu[3][(positionDKJr)].tid, SIGUSR2);
						 	on = false;
 						}
 						else
 						{
 							setGrilleJeu(2,positionDKJr);
					 	 	effacerCarres(11,21,2,2);
				 	 		effacerCarres(10,21,2,2);
					 		setGrilleJeu(3,positionDKJr,DKJR);
					 		afficherDKJr(11,(positionDKJr*2)+7,
					 		((positionDKJr-1)%4)+1);
					 		etatDKJr=LIBRE_BAS;
 						}
 					}
 					// effacerCarres();
 					break;

 			}
 			break;
 			case LIBRE_HAUT :
 			
	 			switch(evenement)
	 			{

	 				case SDLK_LEFT :

	 				if(positionDKJr <= 7 && positionDKJr > 3)
	 				{
	 					if(grilleJeu[1][(positionDKJr)-1].type == CROCO)
	 					{
	 						printf("7\n");
				 			effacerCarres(7,(positionDKJr*2)+ 7,2,2);
				 			setGrilleJeu(1, positionDKJr);
							pthread_kill(grilleJeu[1][(positionDKJr)-1].tid, SIGUSR2);
							on = false;
	 					}
	 					else
	 					{
	 						setGrilleJeu(1, positionDKJr);
					 		effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
					 		positionDKJr--;
					 		setGrilleJeu(1, positionDKJr, DKJR);
					 		afficherDKJr(7, (positionDKJr * 2) + 7, 
					 		((positionDKJr - 1) % 4) + 1);
	 					}
	 				}
	 				else if(positionDKJr == 3) 
	 				{
	 					if(grilleJeu[0][1].type == CLE)
	 					{
	 						pthread_mutex_lock(&mutexDK);
							MAJDK = true;
							pthread_mutex_unlock(&mutexDK);
							pthread_cond_signal(&condDK); //permet de réveiller le thread qui attend sur la variable de condition => ici en locurance threadDK

							setGrilleJeu(1,positionDKJr);
		 					effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
		 					setGrilleJeu(0,positionDKJr,DKJR,threadDKJr);
		 					afficherDKJr(6, (positionDKJr * 2) + 7, 9);
						 	nanosleep(&temps,NULL);

						 	setGrilleJeu(0,positionDKJr);
						 	effacerCarres(5,(positionDKJr*2)+6,3,2);
							effacerCarres(5, 12, 2, 2);
						 	effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
						 	effacerCarres(6, 12, 2, 2);
						 	effacerCarres(7, 11, 2, 2);
						 	effacerCarres(7, 12, 2, 2);
						 	positionDKJr--;
						 	afficherDKJr(6, (positionDKJr * 2) + 7, 
						 	10);

						 	nanosleep(&temp3,NULL);
						 	setGrilleJeu(0,positionDKJr);
						 	effacerCarres(3,11,3,2);
						 	afficherCage(4);
						 	afficherDKJr(0, 0, 11);
						 

						 	nanosleep(&temps2,NULL);
						 	effacerCarres(5, 12, 2, 2);
						 	effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
						 	effacerCarres(6, 10, 2, 2);
						 	effacerCarres(6, 12, 2, 2);
						 	effacerCarres(7, 10, 2, 2);
						 	effacerCarres(7, 11, 2, 2);
						 	effacerCarres(7, 12, 2, 2);
						 	setGrilleJeu(3,(positionDKJr * 2) + 7, 2, 2);
						 	positionDKJr--;
						 	afficherDKJr(11, (positionDKJr * 2) + 7, 
						 	((positionDKJr - 1) % 4) + 1);
		 					etatDKJr=LIBRE_BAS;
		 					pthread_mutex_lock(&mutexScore);
		 					score = score + 10;
		 					MAJScore = true;
		 					pthread_mutex_unlock(&mutexScore);
		 					pthread_cond_signal(&condScore);

		 					for(int i = 1; i<5 ; i++)
							{
								if(grilleJeu[3][i].type == CROCO)
									pthread_kill(grilleJeu[3][i].tid, SIGUSR2);
							}
							for(int i = 0; i<4 ; i++)
							{
								if(grilleJeu[2][i].type == CORBEAU)
									pthread_kill(grilleJeu[2][i].tid, SIGUSR1);
							}

						}
						else if(grilleJeu[0][1].type == VIDE)
						{

						 	setGrilleJeu(1,positionDKJr);
		 					effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
		 					setGrilleJeu(0,positionDKJr,DKJR,threadDKJr);
		 					afficherDKJr(6, (positionDKJr * 2) + 7, 9);
						 	positionDKJr--;
						 	nanosleep(&temps2,NULL);

						 	setGrilleJeu(0,positionDKJr);
						 	effacerCarres(5, 12, 2, 2);
						 	effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
						 	effacerCarres(6, 12, 2, 2);
						 	effacerCarres(7, 11, 2, 2);
						 	effacerCarres(7, 12, 2, 2);
						 	setGrilleJeu(1,positionDKJr,DKJR,threadDKJr);
						 	afficherDKJr(7, (positionDKJr * 2) + 7, 12);
						 	nanosleep(&temps,NULL);
						 	nanosleep(&temp3,NULL);
						 	effacerCarres(5, 12, 2, 2);
						 	effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
						 	effacerCarres(6, 10, 2, 2);
						 	effacerCarres(6, 12, 2, 2);
						 	effacerCarres(7, 10, 2, 2);
						 	effacerCarres(7, 11, 2, 2);
						 	effacerCarres(7, 12, 2, 2);
						 	afficherDKJr(6, (positionDKJr * 2) + 7, 13);
						 	setGrilleJeu(1,positionDKJr);
						 	positionDKJr--;
						 	positionDKJr--;

						 	for(int i = 1; i<5 ; i++)
							{
								if(grilleJeu[3][i].type == CROCO)
									pthread_kill(grilleJeu[3][i].tid, SIGUSR2);
							}
							for(int i = 0; i<4 ; i++)
							{
								if(grilleJeu[2][i].type == CORBEAU)
									pthread_kill(grilleJeu[2][i].tid, SIGUSR1);
							}

						 	on = false;
						} 			
	 				}
	 				break;

	 				case SDLK_RIGHT:

	 				if(positionDKJr < 7 && positionDKJr >= 3)
	 				{
	 					if(grilleJeu[1][(positionDKJr)+1].type == CROCO)
	 					{
	 						printf("8\n");
				 			effacerCarres(7,(positionDKJr*2)+ 7,2,2);
				 			setGrilleJeu(1, positionDKJr);
							pthread_kill(grilleJeu[1][(positionDKJr)+1].tid, SIGUSR2);
							on = false;
	 					}
	 					else
	 					{
	 						setGrilleJeu(1, positionDKJr);
						 	effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
						 	positionDKJr++;
						 	setGrilleJeu(1, positionDKJr, DKJR);
						 	afficherDKJr(7, (positionDKJr * 2) + 7, 
						 	((positionDKJr - 1) % 4) + 1);
	 					}
	 				}
	 				break;

	 				case SDLK_UP:
	 				if(positionDKJr ==6)
	 				{
	 					setGrilleJeu(1,positionDKJr);
				 	 	effacerCarres(7,(positionDKJr*2)+ 7,2,2);		
				 		setGrilleJeu(0,positionDKJr,DKJR);
				 		afficherDKJr(6,(positionDKJr*2)+7,7);
				 		etatDKJr=LIANE_HAUT;

	 				}
	 				if(positionDKJr <5 && positionDKJr >= 3 && positionDKJr != 6) 
	 				{
	 						if(grilleJeu[1][(positionDKJr)-1].type == CROCO)
						 	{ //pour les point
						 		point = true;
						 	}
	 						setGrilleJeu(1,positionDKJr);
				 	 		effacerCarres(7,(positionDKJr*2)+ 7,2,2);		
				 			setGrilleJeu(0,positionDKJr,DKJR);
				 			afficherDKJr(6,(positionDKJr*2)+7, 8);
				 			pthread_mutex_unlock(&mutexGrilleJeu);
						 	nanosleep(&temps4, NULL);
	 						//sleep(1);
						 	if(grilleJeu[1][(positionDKJr)].type == CROCO)
	 						{
	 							printf("9\n");
							 	effacerCarres(6,(positionDKJr*2)+ 7,2,2);
							 	setGrilleJeu(0, positionDKJr);
							 	pthread_kill(grilleJeu[1][(positionDKJr)].tid, SIGUSR2);
							 	on = false;
	 						}
	 						else
	 						{
	 							pthread_mutex_lock(&mutexGrilleJeu);
	 							effacerCarres(6,17,2,2);
					 			effacerCarres(6,15,2,2);
					 	 		effacerCarres(6,13,2,2);
					 	 		afficherDKJr(7,(positionDKJr*2)+7,	((positionDKJr - 1) % 4) + 1);
					 	 		if(point == true)
								{
									pthread_mutex_lock(&mutexScore);
				 					score = score + 1;
				 					MAJScore = true;
				 					pthread_mutex_unlock(&mutexScore);
				 					pthread_cond_signal(&condScore);
									point = false;
								}
						 	}
	 				}
	 				break;

	 				case SDLK_DOWN :

	 				if(positionDKJr == 7)
	 				{
	 					if(grilleJeu[2][(positionDKJr)].type == CORBEAU)
						{
							printf("10\n");
					 		effacerCarres(7,(positionDKJr*2)+ 7,2,2);
					 		setGrilleJeu(1, positionDKJr);
							pthread_kill(grilleJeu[2][(positionDKJr)].tid, SIGUSR1);
							on = false;
						}
						else
						{
							setGrilleJeu(1,positionDKJr);
					 	 	effacerCarres(7,(positionDKJr*2)+ 7,2,2);
					 		setGrilleJeu(2,positionDKJr,DKJR);
					 		afficherDKJr(10,(positionDKJr*2)+7,
					 		5);
					 		etatDKJr=DOUBLE_LIANE_BAS;
						}
	 				}

	 				
	 			}
	 		break;

	 		case LIANE_HAUT :

	 		switch(evenement)
	 		{
	 			case SDLK_DOWN :


	 				if(positionDKJr == 6)
	 				{
	 					if(grilleJeu[1][(positionDKJr)].type == CROCO)
 						{
 							printf("11\n");
						 	effacerCarres(6,(positionDKJr*2)+ 7,2,2);
						 	effacerCarres(7,(positionDKJr*2)+ 7,2,2);
						 	setGrilleJeu(0, positionDKJr);
						 	pthread_kill(grilleJeu[1][(positionDKJr)].tid, SIGUSR2);
						 	on = false;
 						}
 						else
 						{
 							setGrilleJeu(0,positionDKJr);
					 	 	effacerCarres(7,(positionDKJr*2)+ 7,2,2);
					 	 	effacerCarres(6,19,2,2);	
					 		setGrilleJeu(1,positionDKJr,DKJR);
					 		afficherDKJr(7,(positionDKJr*2)+7,
					 		((positionDKJr-1)%4)+1);
					 		etatDKJr=LIBRE_HAUT;
 						}
	 				}
	 				break;
	 		}
	 		break;
 			
 		}
 		pthread_mutex_unlock(&mutexGrilleJeu);l
 		pthread_mutex_unlock(&mutexEvenement);
 		afficherGrilleJeu(); 
 	}

 	pthread_exit(0);

}

void* FctThreadDK(void*)
{
	int cage = 1;
	struct timespec temps = {0,700000000};
	for(int i=1; i<=4 ;i++) //creation de la cage
	{
		afficherCage(i);
	}
	while(1)
	{


		pthread_mutex_lock(&mutexDK);
		
		while(MAJDK == false)
		{
			pthread_cond_wait(&condDK,&mutexDK); 
		}
		MAJDK = false;
		pthread_mutex_unlock(&mutexDK);

	

			switch(cage)
			{
				case 1:

							effacerCarres(2,7,2,2);  
							cage++;
				break;

				case 2:

							effacerCarres(2,9,2,2);						
							cage++;
				break;
				case 3:

							effacerCarres(4,7,2,2);						
							cage++;
				break;

				case 4:

							effacerCarres(4,9,2,3);
							afficherRireDK();
							nanosleep(&temps,NULL);
							effacerCarres(3,8,2,2);
							for(int i=1; i<5 ;i++)
							{
								afficherCage(i);
							}
							cage = 1;
							score = score + 10;
							pthread_mutex_lock(&mutexScore);
							MAJScore =  true;
							pthread_mutex_unlock(&mutexScore);
							pthread_cond_signal(&condScore);
				break;

			}
		
		
		
	}
	

}

void* FctThreadScore(void*)
 {
 		
 		afficherScore(score); //affiche score a zero

 		while(1)
 		{
 				
 				pthread_mutex_lock(&mutexScore);
				
				while(MAJScore == false)
				{
					pthread_cond_wait(&condScore,&mutexScore); 
				}
				afficherScore(score);
				if(score%300 == 0 && score != 0) 
				{ //1 - 3 30
					if(vie == 1)
					{
						effacerCarres(7, 27, 1, 1);
					}
					else if (vie == 2)
					{
						effacerCarres(7, 28, 1, 1);
					}
					vie--;
					pthread_mutex_lock(&mutexDelaiEnemi); // simplifier car tmps que ar 300 , avt 2500
						delaiEnnemis = 3000;
					pthread_mutex_unlock(&mutexDelaiEnemi);
				}
				MAJScore = false;
				pthread_mutex_unlock(&mutexScore);
 		}
 }

 void* FctThreadEnnemis(void*)
 {
 	sigset_t mask;
 	struct timespec temps ;
 	int result;
 	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

 	alarm(15);

  while(1)
 	{
 		int choixEnnemi;
		
		pthread_mutex_lock(&mutexDelaiEnemi);
 		temps.tv_sec = delaiEnnemis / 1000; //4000 / 1000 => 4
		temps.tv_nsec = (delaiEnnemis % 1000) * 1000000; // 4000 % 1000 => 0 * 10000000 => 0
		pthread_mutex_unlock(&mutexDelaiEnemi); 
 		srand(time(0)); 
 		choixEnnemi = (rand()%2)+0; // 0 pour corbeau, 1 pour croco

 		do  // temps reprend bien
		{
			result = nanosleep(&temps, &temps); //du coup le temps qui est interrompu il reste stocké dans temps ? 
		} 
		while (result == -1);

 		if (choixEnnemi == 0)
		{
				printf("CORBEAU\n");
        pthread_create(&threadCorbeau, NULL, FctThreadCorbeau, NULL);

    } 
    else 
    {
    		printf("CROCO\n");
        pthread_create(&threadCroco, NULL, FctThreadCroco, NULL);
    }
 	}
 	/*printf("CROCO\n"); //simple zone de teste pour l'execution des enemi après une mort de DKjr
  pthread_create(&threadCroco, NULL, FctThreadCroco, NULL);
  sleep(1);
  printf("CROCO\n");
  pthread_create(&threadCroco, NULL, FctThreadCroco, NULL);
  sleep(1);
  printf("CROCO\n");
  pthread_create(&threadCroco, NULL, FctThreadCroco, NULL);*/
  pthread_exit(0);
 }
void* FctThreadCroco(void*)
{
	bool end = false;
	int numSuperieur, spriteSuperieur = -1, numInferieur, spriteInferieur = 1;
 	struct timespec intervaleCroco = {0,700000000}; 

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	while(end == false)
	{
		S_CROCO *structpos = (S_CROCO *)pthread_getspecific(keySpec); // val actu de la cle ds struct 
		if(structpos == NULL)
		{
			//on passe ici a chaque nouveau thread 
			structpos = (S_CROCO *)malloc(sizeof(S_CROCO));
			(*structpos).haut = true;
	 		(*structpos).position = 1;
	 		pthread_setspecific(keySpec, (const void *)structpos); // val structds la cle

	 		pthread_mutex_lock(&mutexGrilleJeu);
			setGrilleJeu(1, (*structpos).position, CROCO, pthread_self());
		}

	 	if((*structpos).haut == true)
	 	{
	 		if(grilleJeu[1][(*structpos).position+1].type == DKJR)
	 		{
	 			printf("21\n");
	 			pthread_kill(threadDKJr, SIGHUP); // sighup va kill dkjr
		 		setGrilleJeu(1, (*structpos).position);
		 		effacerCarres(8, ((*structpos).position)*2 + 7, 1, 1);
		 		pthread_mutex_unlock(&mutexGrilleJeu);
		 		pthread_exit(0);
	 		}

	 		if(spriteSuperieur > 0)
		 	{
		 		numSuperieur = 1;
		 	}
		 	else
		 	{
		 		numSuperieur = 2;
		 	}
		 	spriteSuperieur = -spriteSuperieur; // chq tour de bcl chg val de sprite

		 	(*structpos).position++;
	 		if((*structpos).position != 8)
	 		{

	 			setGrilleJeu(1, (*structpos).position, CROCO, pthread_self());
	 			if((*structpos).position > 1)
	 			{
	 				setGrilleJeu(1, ((*structpos).position)-1, VIDE, 0);
	 				effacerCarres(8, (((*structpos).position)*2 + 7)-2, 1, 1);
	 			}
	 			afficherCroco(((*structpos).position)*2 + 7, numSuperieur);
	 		}
	 		else
	 		{
	 			effacerCarres(8, (((*structpos).position)*2 + 7)-2, 1, 1);
	 			setGrilleJeu(1, ((*structpos).position)-1, VIDE, 0);
	 			afficherCroco(((*structpos).position)*2 + 7, 3);
	 			(*structpos).haut = false;
	 		}
	 	}
	 	else
	 	{
	 		if(grilleJeu[3][(*structpos).position-1].type == DKJR)
	 		{
	 			printf("22\n");
	 			pthread_kill(threadDKJr, SIGCHLD);
		 		setGrilleJeu(3, (*structpos).position, VIDE, 0);
		 		if(positionDKJr == 7)
		 		{
		 			effacerCarres(9, 23, 1, 1);
		 		}
		 		else
		 		{
		 			effacerCarres(12, ((*structpos).position)*2 + 8, 1, 1);
		 		}
		 		pthread_mutex_unlock(&mutexGrilleJeu);
		 		pthread_exit(0);
	 		}

	 		if((*structpos).position == 1)
	 		{
	 			setGrilleJeu(3, ((*structpos).position), VIDE, 0);
	 			effacerCarres(12, (((*structpos).position)*2 + 8), 1, 1);
	 			if(positionDKJr == 1 && etatDKJr == LIBRE_BAS) //evité le carré moche (pied dkjr) lors du respawn
	 			{
	 				afficherDKJr(11, 9, 1);
	 			}
			 	end = true;
	 		}
	 		else
	 		{
	 			(*structpos).position--;
	 			if(spriteInferieur > 0)
			 	{
			 		numInferieur = 4;
			 	}
			 	else
			 	{
			 		numInferieur = 5;
			 	}
			 	spriteInferieur = -spriteInferieur;

			 	if((*structpos).position == 7)
			 	{
			 		effacerCarres(9, 23, 1, 1); 
			 	}
			 	else
			 	{
			 		effacerCarres(12, (((*structpos).position)*2 + 8)+2, 1, 1);
			 		setGrilleJeu(3, ((*structpos).position)+1, VIDE, 0);
			 	}
			 	setGrilleJeu(3, (*structpos).position, CROCO, pthread_self());
			 	afficherCroco(((*structpos).position)*2 + 8, numInferieur);
	 		}
	 	}

	 	pthread_mutex_unlock(&mutexGrilleJeu); 
		nanosleep(&intervaleCroco,NULL);
	}
	pthread_exit(0);
}


void* FctThreadCorbeau(void*)
{
		bool nohit = true;
		int num;
 		int sprite = 1;
 		struct timespec intervale = {0,700000000}; 

 		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_UNBLOCK, &mask, NULL);

 		while(nohit == true)
 		{
 			pthread_mutex_lock(&mutexGrilleJeu);

 			int *position = (int *)pthread_getspecific(keySpec); //récupération dans un pointeur de la zone mémoire alloué dynamiquement
	 		if (position == NULL) // 1ere fois thread s'lance
	 		{
	 			position = (int *)malloc(sizeof(int));
	 			*position = 0;
	 			pthread_setspecific(keySpec, (const void *)position);
	 		}
	 		
	 		if(grilleJeu[2][(*position)+1].type == DKJR)
		 	{
		 		printf("23\n");
		 		pthread_kill(threadDKJr, SIGINT);
		 		setGrilleJeu(2, *position);
		 		effacerCarres(9, ((*position)*2+8), 2, 1);
		 		pthread_mutex_unlock(&mutexGrilleJeu);
		 		pthread_exit(0);
		 	}

	 		if(sprite > 0)
	 		{
	 			num = 1;
	 		}
	 		else
	 		{
	 			num = 2;
	 		}
	 		sprite = -sprite;
	 		setGrilleJeu(2, *position, CORBEAU, pthread_self());
	 		(*position)++;
	 		if ((*position) > 7)
	 		{
	 			setGrilleJeu(2, (*position)-1);
	 			effacerCarres(9, (((*position)-1)*2+8), 2, 1);
	 			nohit = false;
	 		}
	 		else
	 		{
	 			setGrilleJeu(2, *position, CORBEAU, pthread_self());
	 			effacerCarres(9, ((*position)*2+8)-2, 2, 1);
	 			setGrilleJeu(2, (*position)-1);
	 			afficherCorbeau((*position)*2+8, num);
	 		}
	 		pthread_mutex_unlock(&mutexGrilleJeu);
	 		nanosleep(&intervale,NULL);	 		// tmps entre chaque bcl
 		}
 		pthread_exit(0);
}

void HandlerSIGQUIT(int sig)
{
	//printf("\nHandler signal : SIGQUIT\n");
	//exit(1);
}

void HandlerSIGUSR1(int sig)
{
	fflush(stdout);
	//printf("\nSIGUSR1 pour le thread d'id(%u)",pthread_self());
	printf("SIGUSR1 pour le thread corbeau d'id(%u)\n",pthread_self());

	int * position = (int *)pthread_getspecific(keySpec); // grâce a var spec , on peut recup la posi

	pthread_mutex_lock(&mutexGrilleJeu);

	setGrilleJeu(2, *position);
	effacerCarres(9, ((*position)*2+8), 2, 1);

	pthread_mutex_unlock(&mutexGrilleJeu);

	pthread_exit(0);
}

void HandlerSIGUSR2(int sig)
{
	fflush(stdout);
	printf("SIGUSR2 pour le thread croco d'id(%u)\n",pthread_self());

	S_CROCO *structpos = (S_CROCO *)pthread_getspecific(keySpec);

	pthread_mutex_lock(&mutexGrilleJeu);


	if((*structpos).haut == true)
	{
		setGrilleJeu(1, (*structpos).position);
		effacerCarres(8, (((*structpos).position)*2+7), 1, 1);
	}
	else
	{
		setGrilleJeu(3, (*structpos).position);
		effacerCarres(12, (((*structpos).position)*2+8), 1, 1);
	}
	
	pthread_mutex_unlock(&mutexGrilleJeu);

	pthread_exit(0);
}
void HandlerSIGINT(int sig)
{
	fflush(stdout);
	printf("SIGINT pour le thread DKJr d'id(%u)",pthread_self());

	setGrilleJeu(2, positionDKJr, VIDE, 0); 

	if(etatDKJr == LIBRE_BAS) // pemret lors d'un saut de recup eveneement , faire d'aitres actions
	{
		pthread_mutex_unlock(&mutexEvenement); 
	}

	effacerCarres(10, (positionDKJr * 2) + 7, 2, 2); //10 car il saute

	pthread_exit(0);
}
void HandlerSIGALRM(int sig)
{
	fflush(stdout);
	printf("RECEPTION DE SIGALRM\n");
	pthread_mutex_lock(&mutexDelaiEnemi);
	delaiEnnemis = delaiEnnemis - 250 ; // ici on fait moins 250 car delaiEnnemis = 4000 (4secondes à la base) et moins 0,25 sec => 250

	if(delaiEnnemis >2500 ) // On cesse de réinitialiser l’alarme quand la variable delaiEnnemis stocke un délai de 2,5 secondes(delai minimum)
	{
		alarm(15);
	}
	pthread_mutex_unlock(&mutexDelaiEnemi);

}
void HandlerSIGHUP(int sig)
{
	fflush(stdout);
	printf("SIGUP pour le thread DKJr d'id(%u)\n",pthread_self());

	setGrilleJeu(1, positionDKJr, 0); // effacé DKjr je c pas si c bon de le faire ici

	effacerCarres(7, (positionDKJr * 2) + 7, 2, 2); 

	pthread_exit(0);

}
void HandlerSIGCHLD(int sig)
{
	fflush(stdout);
	printf("SIGCHLD pour le thread DKJr d'id(%u)\n",pthread_self());

	fflush(stdout);
	setGrilleJeu(3, positionDKJr, 0); // effacé DKjr je c pas si c bon de le faire ici

	effacerCarres(11, (positionDKJr * 2) + 7, 2, 2); 

	pthread_exit(0);
}
void DestructeurVS(void *p)
{
	printf("\nDescruteur de la variable specifique\n");

	fflush(stdout);
	free(p);
}
