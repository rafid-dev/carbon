#include "dataloader.h"
#include <ctime>

namespace DataLoader {
    void DataSetLoader::loadNextBatch() {
        positionIndex += batchSize;

        if (positionIndex == CHUNK_SIZE) {
            // Join thread that's reading nextData
            if (readingThread.joinable()) {
                readingThread.join();
            }

            // Bring next data to current position
            std::swap(currentData, nextData);
            positionIndex = 0;

            // Begin a new thread to read nextData
            readingThread = std::thread(&DataSetLoader::loadNext, this);
        }
    }

    void DataSetLoader::loadNext() {
        for (std::size_t counter = 0; counter < CHUNK_SIZE; ++counter) {
            // If we finished, go back to the beginning
            if (!reader.hasNext()) {
                reader = binpack::CompressedTrainingDataEntryReader(path);
            }

            DataSetEntry& positionEntry = nextData[permuteShuffle[counter]];

            // Get info
            positionEntry.entry = reader.next();

            bool earlySkip = positionEntry.entry.ply <= 16;
            bool filter = positionEntry.entry.isInCheck() || positionEntry.entry.isCapturingMove();

            if (positionEntry.entry.score == 32002) {
                counter--;
                continue;
            }
            
            if (earlySkip || filter) {
                counter--;
                continue;
            }
        }
    }

    void DataSetLoader::shuffle() {
        std::mt19937 mt{static_cast<std::mt19937::result_type>(69)};
        std::iota(permuteShuffle.begin(), permuteShuffle.end(), 0);
        std::shuffle(permuteShuffle.begin(), permuteShuffle.end(), mt);
    }

    void DataSetLoader::init() {
        positionIndex = 0;

        shuffle();

        loadNext();
        std::swap(currentData, nextData);
        loadNext();
    }
} // namespace DataLoader