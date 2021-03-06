//===- VanillaBackend.h -------------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_VANILLA_VANILLA_BACKEND_H
#define TARGET_VANILLA_VANILLA_BACKEND_H
#include <string>
#include <onnc/Target/TargetBackend.h>

namespace onnc {

class VanillaBackend : public TargetBackend
{
public:
  VanillaBackend(const TargetOptions& pOptions);

  virtual ~VanillaBackend() = default;

  void addTensorSel(PassManager& pPM) override;

  void addTensorSched(PassManager& pPM) override;
  
  void addMemAlloc(PassManager& pPM) override;

  void addCodeEmit(PassManager& pPM, const Path& pOutput) override;

  void RegisterLowers(LowerRegistry& pRegistry) const override;
};

}  // namespace onnc

#endif
