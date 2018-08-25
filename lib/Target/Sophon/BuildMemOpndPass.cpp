//===- BuildMemOpndPass.cpp -----------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "tg_build_memop"
#include "BuildMemOpndPass.h"
#include <onnc/Core/ModulePass.h>
#include <onnc/Core/PassSupport.h>
#include <onnc/IR/Compute/Cast.h>
#include <onnc/IR/Compute/Initializer.h>
#include <onnc/IR/Compute/Reshape.h>
#include <onnc/Support/Debug.h>
#include <onnc/Support/IOStream.h>

using namespace onnc;

char BuildMemOpnd::ID = 0;

//===----------------------------------------------------------------------===//
// BuildMemOpnd
//===----------------------------------------------------------------------===//
BuildMemOpnd::BuildMemOpnd()
    : ModulePass(ID)
{
}

Pass::ReturnType BuildMemOpnd::runOnModule(::onnc::Module &pModule)
{
  clear();

  Module::cg_iterator cg, cgEnd = pModule.cgEnd();
  for (cg = pModule.cgBegin(); cg != cgEnd; ++cg)
    createAllMemOperands(*cg->value());

  return Pass::kModuleNoChanged;
}

void BuildMemOpnd::createAllMemOperands(onnc::ComputeGraph &pCG)
{
  ComputeGraph::iterator nodeIt, nEnd = pCG.end();
  for (nodeIt = pCG.begin(); nodeIt != nEnd; ++nodeIt) {
    ComputeOperator *node = nodeIt;
    ComputeOperand::Residence resd = ComputeOperand::kOutputResidence;
    if (isa<Initializer>(node))
      resd = ComputeOperand::kWeightResidence;

    createMemOperandsOfNode(pCG, *node, resd);
  }
}

void BuildMemOpnd::createMemOperandsOfNode(ComputeGraph &pCG,
                                           ComputeOperator &pNode,
                                           ComputeOperand::Residence pResd)
{
  // no-op operator, input value's MemOperand is same as output's
  if (isa<Reshape>(&pNode)) {
    onnc::Value *inputValue = pNode.getInput(0);
    onnc::Value *outputValue = pNode.getOutput(0);
    // find input's MemOperand
    ComputeMemOperand *inputCMO = nullptr;
    for (auto pair : m_ValOperandMap) {
      if (pair.second == inputValue) {
        inputCMO = pair.first;
        break;
      }
    }
    assert(inputCMO != nullptr);
    m_ValOperandMap.emplace_back(inputCMO, outputValue);
    DEBUG(dbgs() << "insert ValOperandMap: " << inputCMO << ","
                 << outputValue->getName() << "\n");
    return;
  }
  unsigned int out_size = pNode.getNumOfOutputs();
  for (unsigned int i = 0; i < out_size; ++i) {
    onnc::Value* value = pNode.getOutput(i);
    onnc::Value::UseList& use_list = value->getUses();
    onnc::Value::UseList::iterator use, uEnd = use_list.end();
    for (use = use_list.begin(); use != uEnd; ++use) {
      ComputeMemOperand *memOperand =
        pCG.addOperand<ComputeMemOperand>(pNode, *use->getUser(),
                                          *value, pResd);
      m_ValOperandMap.emplace_back(memOperand, value);
      DEBUG(dbgs() << "insert ValOperandMap: " << memOperand << ","
                   << value->getName() << "\n");
    }
  }
}

void BuildMemOpnd::clear()
{
  m_ValOperandMap.clear();
}

//===----------------------------------------------------------------------===//
// Factory method
//===----------------------------------------------------------------------===//
ModulePass *onnc::CreateBuildMemOpndPass()
{
  return new BuildMemOpnd();
}