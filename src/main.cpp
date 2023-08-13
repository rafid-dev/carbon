#include "argparse.h"
#include "trainer.h"
#include "quantize.h"

#include <omp.h>
#include <sstream>

int main(int argc, char* argv[]) {
    ArgumentParser parser;
    parser.addArgument("--dataset", "Path to the dataset.");
    parser.addArgument("--epochs", "Number of epochs to train for.");
    parser.addArgument("--id", "Network ID. Leave for random. Use '$' for a random number placeholder.", true);
    parser.addArgument("--lr", "Learning rate. (Default 0.001)", true);
    parser.addArgument("--checkpoint", "Path to the checkpoint to load from.", true);
    parser.addArgument("--savepath", "Path to where checkpoints will be saved.", true);
    parser.setProgramName(argv[0]);

    // Print help and exit if no arguments or --help flag provided
    if (argc == 1 || (argc == 2 && std::string(argv[1]) == "--help")) {
        parser.printHelp();
        return 0;
    }

    // Parse arguments
    if (!parser.parse(argc, argv)) {
        return 1;
    }

    // Extract values from parsed arguments
    std::string datasetPath    = parser.getArgumentValue("--dataset");
    std::string checkpointPath = parser.getArgumentValue("--checkpoint");
    std::string savepath       = parser.getArgumentValue("--savepath");
    std::string networkId      = parser.getArgumentValue("--id");
    int         epochs         = std::stoi(parser.getArgumentValue("--epochs"));
    float       lr             = parser.getArgumentValue("--lr").empty() ? 0.001f : std::stof(parser.getArgumentValue("--lr"));

    Trainer* trainer = new Trainer{datasetPath, 16384};

    // Configure trainer
    trainer->setNetworkId(networkId);
    trainer->setMaxEpochs(epochs);
    trainer->setSavePath(savepath);
    trainer->setLearningRate(lr);

    // Print Configurations
    std::cout << "Number of Available Threads: " << omp_get_max_threads() << "\n";
    std::cout << "Allocated threads: " << THREADS << "\n";
    std::cout << "Dataset Path: " << datasetPath << "\n";
    std::cout << "Checkpoint Path: " << checkpointPath << "\n";
    std::cout << "Save Path: " << savepath << "\n";
    std::cout << "Network ID: " << trainer->getNetworkId() << "\n\n";
    std::cout << "Learning Rate: " << trainer->getLearningRate() << "\n";
    std::cout << "Optimizer: " << trainer->optimizer << "\n";
    std::cout << "LR Scheduler: " << trainer->lrScheduler << "\n";
    std::cout << "Epochs: " << trainer->getMaxEpochs() << "\n";
    std::cout << "Batchsize: " << trainer->getBatchSize() << "\n\n";

    if (!checkpointPath.empty()) {
        std::cout << "Loading checkpoint from " << checkpointPath << std::endl;
        trainer->loadCheckpoint(checkpointPath);
        std::cout << "Loaded checkpoint from " << checkpointPath << std::endl;
    }

    std::cout << std::endl;
    
    trainer->train();

    return 0;
}