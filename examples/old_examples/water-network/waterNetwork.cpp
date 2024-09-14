#include <iostream>
#include <stdio.h>
#include <string.h>
#include <filesystem>
#include <unistd.h>
#include "cmath"

#include "Environment.h"
#include "Function.h"
#include "LinearFunction.h"
#include "CGraph.h"
#include "OpCode.h"
#include "Problem.h"
#include "IpoptEngine.h"
#include "FilterSQPEngine.h"
#include "Glob.h"


using namespace Minotaur;
using namespace std;


struct ptr
{
  int data;
  struct ptr *next;
};

void append(struct ptr** head_ref, int new_data)
{
    struct ptr* new_ptr = (struct ptr*) malloc(sizeof(struct ptr));
    struct ptr *last = *head_ref;
    new_ptr->data  = new_data;
    new_ptr->next = NULL;
    if (*head_ref == NULL)
    {
       *head_ref = new_ptr;
       return;
    } 
    while (last->next != NULL)
        last = last->next;
    last->next = new_ptr;
    return;   
}

void printList(struct ptr *node)
{
  while (node != NULL)
  {
     printf(" %d ", node->data);
     node = node->next;
  }
}

int main()
{
  char file[] = "/Data/Sample_input_cycle_twoloop.dat";
  // char file[] = "/Data/HG_SP_4_2.dat";
  char tmp[256];
  getcwd(tmp, 256);
  cout<<tmp<<endl;
  char * address = new char[strlen(tmp)+strlen(file)+1];
  strcpy(address,tmp);
  strcat(address,file);
  cout<<"Running ~"<<address<<endl;

  map<int,array<int,2>> arcs;
  map<array<int,2>, float> Length;
  map<int,double> d;
  map<int,float> E;
  map<int,float> prs;
  map<int,float> dem;
  map<int,float> Cost;
  map<int,float> R;
  map<array<int,2>, VariablePtr> q1;
  map<array<int,2>, VariablePtr> q2;
  map<int, VariablePtr> h;
  map<array<int,3>, VariablePtr> l;

  FILE *fptr;
  if ((fptr = fopen(address, "r")) == NULL) {
      printf("Error! File cannot be opened.");
      exit(1);
  }
  
  char c[1000];
  char * token;
  bool flag =1;
  
  struct ptr* vertex = NULL;
  fgets(c, 1000, (FILE*)fptr);
  token = strtok(c," ");
  while(token!=NULL){
    if(atoi(token)!=0){
      append(&vertex,atoi(token));
    }
    token = strtok(NULL," ");
  }
  // printf("Set of Nodes:");
  // printList(vertex);printf("\n");
  
  struct ptr* pipes = NULL;
  fgets(c, 1000, (FILE*)fptr);
  token = strtok(c," ");
  while(token!=NULL){
    if(atoi(token)!=0){
      append(&pipes,atoi(token));
    }
    token = strtok(NULL," ");
  }
  // printf("Set of Pipes:");
  // printList(pipes);printf("\n");
  
  int count=0;
  struct ptr* arc1=NULL;
  struct ptr* arc2=NULL;
  struct ptr* L=NULL;
  while(fgets(c, 1000, (FILE*)fptr)!=NULL){
    if (flag==1){
      flag = 0;
    }
    else{
      token = strtok(c," ");
      if (!isalnum(*token)) { break;}
      while(token!=NULL){
        if(atoi(token)!=0){
          if(count%3==0){
            append(&arc1,atoi(token));
            count++;
          }
          else if(count%3==1){
            append(&arc2,atoi(token));
            count++;
          }
          else if(count%3==2){
            append(&L,atoi(token));
            count++;
          }
        }
        token = strtok(NULL," ");
      }
    }
  }

  fgets(c, 1000, (FILE*)fptr);
  count=0;
  struct ptr* ppe = pipes;
  while(fgets(c, 1000, (FILE*)fptr)!=NULL){
    if (flag==1){
      flag = 0;
    }
    else{
      token = strtok(c," ");
      if (!isalnum(*token)) { break;}
      while(token!=NULL){
        if(atoi(token)!=0){
          if(count%2==0){
            count++;
          }
          else if(count%2==1){
            d.insert(pair<int,float>(ppe->data,atof(token)));
            ppe = ppe->next;
            count++;
          }
        }
        token = strtok(NULL," ");
      }
    }
  }
  // for (auto iter = d.begin();iter!=d.end();++iter)
  //   cout<<"d"<<iter->first<<" : "<<iter->second<<" mm\n";
  
  fgets(c, 1000, (FILE*)fptr);
  count=0;
  struct ptr *n = vertex;
  while(fgets(c, 1000, (FILE*)fptr)!=NULL){
    if (flag==1){
      flag = 0;
    }
    else{
      token = strtok(c," ");
      if (!isalnum(*token)) { break;}
      while(token!=NULL){
        if(atoi(token)!=0){
          if(count%2==0){
            count++;
          }
          else if(count%2==1){
            E.insert(pair<int,float>(n->data,atof(token)));
            n=n->next;
            count++;
          }
        }
        token = strtok(NULL," ");
      }
    }
  }
  // for (auto iter = E.begin();iter!=E.end();++iter)
  //   cout<<"E_"<<iter->first<<" : "<<iter->second<<" m\n";

  fgets(c, 1000, (FILE*)fptr);
  count=0;
  n = vertex;
  while(fgets(c, 1000, (FILE*)fptr)!=NULL){
    if (flag==1){
      flag = 0;
    }
    else{
      token = strtok(c," ");
      if (!isalnum(*token)) { break;}
      while(token!=NULL){
        if(count%2==0){
          count++;
        }
        else if(count%2==1){
          prs.insert(pair<int,float>(n->data,atof(token)));
          n=n->next;
          count++;
        }
        token = strtok(NULL," ");
      }
    }
  }
  // for (auto iter = prs.begin();iter!=prs.end();++iter)
  //   cout<<"Pressure_"<<iter->first<<" : "<<iter->second<<" m\n";

  fgets(c, 1000, (FILE*)fptr);
  count=0;
  n = vertex;
  while(fgets(c, 1000, (FILE*)fptr)!=NULL){
    if (flag==1){
      flag = 0;
    }
    else{
      token = strtok(c," ");
      if (!isalnum(*token)) { break;}
      while(token!=NULL){
        if(count%2==0){
          count++;
        }
        else if(count%2==1){
          dem.insert(pair<int,float>(n->data,atof(token)));
          n = n->next;
          count++;
        }
        token = strtok(NULL," ");
      }
    }
  }

  float q_M=0;
  for (auto iter = dem.begin();iter!=dem.end();++iter){
    // cout<<"Demand at node "<<iter->first<<" = "<<iter->second<<" liter/sec\n";
    if(abs(iter->second)>q_M)
      q_M = abs(iter->second);
  }

  fgets(c, 1000, (FILE*)fptr);
  count=0;
  ppe = pipes;
  while(fgets(c, 1000, (FILE*)fptr)!=NULL){
    if (flag==1){
      flag = 0;
    }
    else{
      token = strtok(c," ");
      if (!isalnum(*token)) { break;}
      while(token!=NULL){
        if(count%2==0){
          count++;
        }
        else if(count%2==1){
          Cost.insert(pair<int,float>(ppe->data,atof(token)));
          count++;
          ppe = ppe->next;
        }
        token = strtok(NULL," ");
      }
    }
  }
  // for (auto iter = Cost.begin();iter!=Cost.end();++iter)
  //   cout<<"Cost of pipe "<<iter->first<<" = $"<<iter->second<<"\n";
  
  fgets(c, 1000, (FILE*)fptr);
  count=0;
  ppe = pipes;
  while(fgets(c, 1000, (FILE*)fptr)!=NULL){
    if (flag==1){
      flag = 0;
    }
    else{
      token = strtok(c," ");
      if (!isalnum(*token)) { break;}
      while(token!=NULL){
        if(count%2==0){
          count++;
        }
        else if(count%2==1){
          R.insert(pair<int,float>(ppe->data,atof(token)));
          count++;
          ppe = ppe->next;
        }
        token = strtok(NULL," ");
      }
    }
  }
  // for (auto iter = R.begin();iter!=R.end();++iter)
  //   cout<<"Pipe roughness coefficient of pipe "<<iter->first<<" "<<iter->second<<"\n";
  
  count=0;
  struct ptr* Source=NULL;
  while(fgets(c, 1000, (FILE*)fptr)!=NULL){
    token = strtok(c," ");
    if (!isalnum(*token)) { break;}
    while(token!=NULL){
      if(count%3==2){
        append(&Source,atoi(token)); 
        count++;
      }
      else{
        count++;
      }
      token = strtok(NULL," ");
    }
  }

  Minotaur::EnvPtr env(new Minotaur::Environment());

  // Write full version.
  cout << env->getVersion() << endl;

  // Greetings.
  // cout << endl << "Hello World" << endl;

  Problem p(env);

  struct ptr *a1 = arc1;
  struct ptr *a2 = arc2;
  struct ptr *pp = pipes;
  struct ptr *ll = L;
  int counter = 0;
  while (a1 != NULL)
  {
     pp = pipes;
     arcs.insert(pair<int,array<int,2>>(counter,{a1->data,a2->data}));
     q1.insert(pair<array<int,2>,VariablePtr>({a1->data,a2->data},p.newVariable(0.00000001,q_M,Continuous,"q1_"+to_string(a1->data)+"_"+to_string(a2->data))));
     q2.insert(pair<array<int,2>,VariablePtr>({a1->data,a2->data},p.newVariable(0.00000001,q_M,Continuous,"q2_"+to_string(a1->data)+"_"+to_string(a2->data))));
     Length.insert(pair<array<int,2>,float>({a1->data,a2->data},ll->data));
     while (pp!=NULL){
      l.insert(pair<array<int,3>,VariablePtr>({a1->data,a2->data,pp->data},p.newVariable(0,ll->data,Continuous,"l_"+to_string(a1->data)+"_"+to_string(a2->data)+"_"+to_string(pp->data))));
      pp = pp->next;
     }
     a1 = a1->next;
     a2 = a2->next;
     ll = ll->next;
     counter++;
  }
  LinearFunctionPtr lf = new LinearFunction();

  for(auto iter = arcs.begin();iter!=arcs.end();++iter){
    ppe = pipes;
    while(ppe!=NULL){
      lf->addTerm(l[{iter->second[0],iter->second[1],ppe->data}],Cost[ppe->data]);
      ppe = ppe->next;
    }
  }
  FunctionPtr f = new Function(lf);
  p.newObjective(f, 0.0, Minimize);

  // Demand at each node is being met or not
  n = vertex;
  while (n != NULL){
    h.insert(pair<int,VariablePtr>(n->data,p.newVariable(E[n->data]+prs[n->data],E[Source->data],Continuous,"h_"+to_string(n->data))));
    lf = new LinearFunction();
    for(auto iter=arcs.begin();iter!=arcs.end();++iter){
      if(iter->second[1]==n->data){
        lf->addTerm(q1[iter->second],1.0);
        lf->addTerm(q2[iter->second],-1.0);
      }
      else if(iter->second[0]==n->data){
        lf->addTerm(q1[iter->second],-1.0);
        lf->addTerm(q2[iter->second],1.0);
      }
    }  
    f = new Function(lf);
    p.newConstraint(f,dem[n->data],dem[n->data],"Demand_"+to_string(n->data));
    n = n->next;
  }

  //Summation of each segment of pipe equals to total link length
  for(auto iter=arcs.begin();iter!=arcs.end();++iter){
    pp = pipes;
    lf = new LinearFunction();
    while(pp!=NULL){
      lf->addTerm(l[{iter->second[0],iter->second[1],pp->data}],1.0);
      pp = pp->next;
    }
    f = new Function(lf);
    p.newConstraint(f,Length[{iter->second[0],iter->second[1]}],Length[{iter->second[0],iter->second[1]}],"L_"+to_string(iter->second[0])+"_"+to_string(iter->second[1]));
  }

  // Head Loss equality constraint
  CGraph * nlf;
  CNode *c0,*c1,*c2,*c3;
  for(auto iter=arcs.begin();iter!=arcs.end();++iter){
    nlf = new CGraph();
    pp = pipes;
    c2 = nlf->newNode(0.001);
    c1 = nlf->newNode(q1[iter->second]);
    c1 = nlf->newNode(OpMult,c1,c2);
    c3 = nlf->newNode(q2[iter->second]);
    c3 = nlf->newNode(OpMult,c3,c2);
    c2 = nlf->newNode(1.852);
    c1 = nlf->newNode(OpPowK,c1,c2);
    c3 = nlf->newNode(OpPowK,c3,c2);
    c3 = nlf->newNode(OpMinus,c1,c3);
    
    // c0 = nlf->newNode(OpMult,c0,c1);
    c0 = nlf->newNode(0);

    while(pp!=NULL){
      c1 = nlf->newNode(l[{iter->second[0],iter->second[1],pp->data}]);
      c2 = nlf->newNode(10.67/((pow(R[pp->data],1.852)*pow(d[pp->data]/1000.0,4.87))));
      c1 = nlf->newNode(OpMult,c1,c2);
      c1 = nlf->newNode(OpMult,c1,c3);
      c0 = nlf->newNode(OpPlus,c0,c1);
      pp = pp->next;
    }
    
    // c1 = nlf->newNode(OpAbs,c1,NULL);
    // c0 = nlf->newNode(OpMult,c0,c1);
    c1 = nlf->newNode(h[iter->second[0]]);
    c2 = nlf->newNode(h[iter->second[1]]);
    c1 = nlf->newNode(OpMinus,c1,c2);
    c0 = nlf->newNode(OpMinus,c1,c0);
    nlf->setOut(c0);
    nlf->finalize();
    // nlf->write(std::cout);
    // cout<<"\n";
    f = new Function(nlf);
    // f->write(cout);
    p.newConstraint(f,0,0,"HL_"+to_string(iter->second[0])+"_"+to_string(iter->second[1]));
  }

  // Adding q1*q2 <= delta
  for(auto iter=arcs.begin();iter!=arcs.end();++iter){
    nlf = new CGraph();
    c0 = nlf->newNode(q1[iter->second]);
    c1 = nlf->newNode(q2[iter->second]);
    c0 = nlf->newNode(OpMult,c0,c1);
    nlf->setOut(c0);
    nlf->finalize();
    f = new Function(nlf);
    p.newConstraint(f,0,q_M*0.00000001,"q1q2_"+to_string(iter->second[0])+"_"+to_string(iter->second[1]));
  }

  // p.write(cout);
  p.setNativeDer();
  p.prepareForSolve();




  // Using Global Solver
  // int err = 0;
  // Glob *solver = new Glob(env);
  // env->startTimer(err);
  // solver->solve(&p);

  // Usign IPOPT
  EnginePtr e = new IpoptEngine(env);
  e->setIterationLimit(4000);
  e->load(&p);
  e->solve();
  cout<<e->getStatusString()<<"\n";
  // e->getSolution()->write(cout);
 
  // p.write(cout);
  return 0;
}