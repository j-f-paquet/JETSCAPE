// -----------------------------------------
// JetScape (modular/task) based framework
// Intial Design: Joern Putschke (2017)
//                (Wayne State University)
// -----------------------------------------
// License and Doxygen-like Documentation to be added ...

#ifndef HARDPROCESS_H
#define HARDPROCESS_H

#include "InitialState.h"
#include "JetScapeModuleBase.h"
#include "tinyxml2.h"
#include "JetClass.hpp"
#include <vector>

namespace Jetscape {

class HardProcess : public JetScapeModuleBase 
{
  
 public:
  
  HardProcess();
  HardProcess(string m_name) : JetScapeModuleBase (m_name)
    {SetId("HardProcess");}
  virtual ~HardProcess();

  virtual void Init();
  virtual void Exec();
  virtual void Clear();

  virtual void WriteTask(weak_ptr<JetScapeWriter> w);
  
  tinyxml2::XMLElement* GetHardXML() {return fd;}

  // connect the InitialState module with hard process
  std::shared_ptr<InitialState> ini;
 
  int GetNHardPartons() {return hp_list.size();}
  shared_ptr<Parton> GetPartonAt(int i) {return hp_list[i];}
  vector<shared_ptr<Parton>>& GetPartonList() {return hp_list;}
  
  void AddParton(shared_ptr<Parton> p) {hp_list.push_back(p);}
  
  // Slots ...
  void GetHardPartonList(vector<shared_ptr<Parton>> &plist) {plist=hp_list;}
  
 private:

  tinyxml2::XMLElement *fd;

  // Think of always using unique_ptr for any vector in jetscape framework !???
  // To be discussed ...
  vector<shared_ptr<Parton>> hp_list;

 
};

} // end namespace Jetscape

#endif