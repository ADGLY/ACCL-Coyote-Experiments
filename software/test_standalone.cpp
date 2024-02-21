#include <iostream>
#include <string>

#include "cProcess.hpp"

using namespace std;
using namespace fpga;

int main(int argc, char *argv[]) {

  std::unique_ptr<cProcess> cproc;

  const uint64_t max_size = 1ULL << 17ULL;
  uint32_t n_pages = ((max_size + pageSize - 1) / pageSize);

  // Obtain resources
  cproc = std::make_unique<cProcess>(0, getpid());
  int8_t *hMem =
      static_cast<int8_t *>(cproc->getMem({CoyoteAlloc::REG_4K, n_pages}));
  if (!hMem) {
    std::cout << "Unable to allocate mem" << std::endl;
    return 1;
  }

  memset(hMem, 0, max_size);

  const auto nbInputs = 127ULL;
  const auto inputSize = 16ULL;

  const auto inferenceDataSize2D = nbInputs * inputSize;
  for (uint64_t i = 0; i < nbInputs; ++i) {
    for (uint64_t j = 0; j < inputSize; ++j) {
	    if (i % 2 == 0) {
      		hMem[i * inputSize + j] = -1;
	    } else {
      		hMem[i * inputSize + j] = 0;
	    }
    }
  }

  std::cout << "Setting the number of batches to: " << nbInputs << std::endl;

  uint64_t FINN_CTRL_OFFSET = 0x0;

  cproc->setCSR(nbInputs, (FINN_CTRL_OFFSET + 0x10) >> 3);
  cproc->setCSR(0x8110, (FINN_CTRL_OFFSET + 0x18) >> 3);
  cproc->setCSR(1, (FINN_CTRL_OFFSET + 0x0) >> 3);
  while (!(cproc->getCSR((FINN_CTRL_OFFSET + 0x0) >> 3) & 2))
    ;

  std::cout << "Before kernel, data in write region is:" << std::endl;
  for (uint64_t i = inferenceDataSize2D; i < inferenceDataSize2D + nbInputs;
       ++i) {
    std::cout << "hMem[" << i << "] = " << std::to_string(hMem[i]) << std::endl;
  }

  csInvoke invokeRead = {
      .oper = CoyoteOper::READ, .addr = &hMem[0], .len = inferenceDataSize2D};

  csInvoke invokeWrite = {.oper = CoyoteOper::WRITE,
                          .addr = &hMem[inferenceDataSize2D],
                          .len = nbInputs};
  // Sending data to kernel
  std::cout << "Writing to FPGA" << std::endl;
  cproc->invoke(invokeRead);
  std::cout << "Writing to FPGA done" << std::endl;

  // Reading data from kernel
  std::cout << "Reading from host" << std::endl;
  cproc->invoke(invokeWrite);
  std::cout << "Reading from host done" << std::endl;

  std::cout << "After kernel, data in write region is:" << std::endl;
  for (uint64_t i = inferenceDataSize2D; i < inferenceDataSize2D + nbInputs;
       ++i) {
    std::cout << "hMem[" << i << "] = " << std::to_string(hMem[i]) << std::endl;
  }

  return EXIT_SUCCESS;
}
