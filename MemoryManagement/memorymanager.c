#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fifo(int frames[], int requests[], int numRequests, int numFrames,
          FILE *outputFile);
void optimal(int frames[], int requests[], int numRequests, int numFrames,
             FILE *outputFile);
void lru(int frames[], int requests[], int numRequests, int numFrames,
         FILE *outputFile);
int isInFrame(int frames[], int pageRequest, int numFrames);
int findFarthestUse(int frames[], int requests[], int numRequests,
                    int currentRequest, int numFrames);
int findMinIndex(int times[], int numTimes);
int main() {
  FILE *inputFile = fopen("input.txt", "r");
  FILE *outputFile = fopen("output.txt", "w");
  if (inputFile == NULL || outputFile == NULL) {
    printf("Error oepning input/output file");
    exit(1);
  }
  // Read in first line
  // First number is num of pages
  // Second is num of frames
  // Third is num of requests
  int numPages = 0;
  int numFrames = 0;
  int numRequests = 0;
  fscanf(inputFile, "%d %d %d", &numPages, &numFrames, &numRequests);
  int frames[numFrames];
  int requests[numRequests];
  // Set all frame numbers to -1
  for (int i = 0; i < numFrames; i++) {
    frames[i] = -1;
  }
  // Read in the rest of the page requests
  for (int i = 0; i < numRequests; i++) {
    int nextRequest = 0;
    fscanf(inputFile, "%d", &nextRequest);
    requests[i] = nextRequest;
  }

  /*
   * Test that input works
  printf("%d %d %d\n",numPages,numFrames,numRequests);
  for(int i = 0; i < numRequests; i++){
          printf("%d\n",requests[i]);
  }*/
  fprintf(outputFile, "FIFO\n");
  fifo(frames, requests, numRequests, numFrames, outputFile);

  for (int i = 0; i < numFrames; i++) {
    frames[i] = -1;
  }
  fprintf(outputFile, "\nOptimal\n");
  optimal(frames, requests, numRequests, numFrames, outputFile);

  for (int i = 0; i < numFrames; i++) {
    frames[i] = -1;
  }
  fprintf(outputFile, "\nLRU\n");
  lru(frames, requests, numRequests, numFrames, outputFile);

  return 0;
}
void fifo(int frames[], int requests[], int numRequests, int numFrames,
          FILE *outputFile) {
  // If request is not in a frame, add it
  // Else print its already there
  // First head starts at 0, if that one changes, move it forward one
  int counter = 0;
  int pageFaults = 0;
  for (int i = 0; i < numRequests; i++) {
    int frameNumber = isInFrame(frames, requests[i], numFrames);
    if (frameNumber != -1) {
      fprintf(outputFile, "Page %d already in Frame %d\n", requests[i],
              frameNumber);
    } else {
      int currentFrame = counter % numFrames;
      if (frames[currentFrame] == -1) {
        // Empty
        fprintf(outputFile, "Page %d loaded into Frame %d\n", requests[i],
                currentFrame);
        frames[currentFrame] = requests[i];
      } else {
        // Have to unload
        fprintf(
            outputFile,
            "Page %d unloaded from Frame %d, Page %d loaded into Frame %d\n",
            frames[currentFrame], currentFrame, requests[i], currentFrame);
        frames[currentFrame] = requests[i];
      }
      counter++;
      pageFaults++;
    }
  }
  fprintf(outputFile, "%d page faults\n", pageFaults);
}
void optimal(int frames[], int requests[], int numRequests, int numFrames,
             FILE *outputFile) {
  // If frames isn't full, add it
  // If it is find the page that is used farthest in the future and replace
  bool isFull = false;
  int counter = 0;
  int pageFaults = 0;
  for (int i = 0; i < numRequests; i++) {
    int frameNumber = isInFrame(frames, requests[i], numFrames);
    if (frameNumber != -1) {
      fprintf(outputFile, "Page %d already in Frame %d\n", requests[i],
              frameNumber);
    } else {
      if (isFull == false) {
        fprintf(outputFile, "Page %d loaded into Frame %d\n", requests[i],
                counter);
        frames[counter] = requests[i];
        counter++;
        if (counter >= numFrames) {
          isFull = true;
        }
      } else {
        // printf("Find a spot for %d\n",requests[i]);
        int replaceIndex =
            findFarthestUse(frames, requests, numRequests, i, numFrames);
        fprintf(
            outputFile,
            "Page %d unloaded from Frame %d, Page %d loaded into Frame %d\n",
            frames[replaceIndex], replaceIndex, requests[i], replaceIndex);
        frames[replaceIndex] = requests[i];
      }
      pageFaults++;
    }
  }
  fprintf(outputFile, "%d page faults\n", pageFaults);
}
void lru(int frames[], int requests[], int numRequests, int numFrames,
         FILE *outputFile) {
  // Have a second int[], size of frames, that holds the time that each element
  // was accessed Time will be dependent on an int counter, increments at the
  // end of each loop Frame with the lowest time will be choosen
  int timer = 1;
  int times[numFrames];
  int pageFaults = 0;
  for (int i = 0; i < numFrames; i++) {
    times[i] = 0;
  }
  for (int i = 0; i < numRequests; i++) {
    int frameNumber = isInFrame(frames, requests[i], numFrames);
    if (frameNumber != -1) {
      fprintf(outputFile, "Page %d already in Frame %d\n", requests[i],
              frameNumber);
      times[frameNumber] = timer;
    } else {
      // Find min access time index
      int lastAccessed = findMinIndex(times, numFrames);
      if (frames[lastAccessed] == -1) {
        fprintf(outputFile, "Page %d loaded into Frame %d\n", requests[i],
                lastAccessed);
      } else {
        fprintf(
            outputFile,
            "Page %d unloaded from Frame %d, Page %d loaded into Frame %d\n",
            frames[lastAccessed], lastAccessed, requests[i], lastAccessed);
      }
      // Replace that index
      frames[lastAccessed] = requests[i];
      times[lastAccessed] = timer;
      pageFaults++;
    }
    timer++;
  }
  fprintf(outputFile, "%d page faults\n", pageFaults);
}
int findMinIndex(int times[], int numTimes) {
  int minIndex = 0;
  for (int i = 0; i < numTimes; i++) {
    if (times[i] < times[minIndex]) {
      minIndex = i;
    }
  }
  return minIndex;
}
int isInFrame(int frames[], int pageRequest, int numFrames) {
  for (int i = 0; i < numFrames; i++) {
    if (frames[i] == pageRequest) {
      return i;
    }
  }
  return -1;
}
int findFarthestUse(int frames[], int requests[], int numRequests,
                    int currentRequest, int numFrames) {
  int furthestIndex = 0;
  int furthestNum = 0;
  // Loop through each frame
  // Find how many steps away that frame value is in the requests
  // Return the frame index of the farthest
  for (int i = 0; i < numFrames; i++) {
    int stepsToNext = -1;
    for (int j = currentRequest + 1; j < numRequests; j++) {
      stepsToNext++;
      if (frames[i] == requests[j]) {
        break;
      }
      // Hit the end, set to infinity
      if (j == numRequests - 1) {
        stepsToNext = -1;
      }
    }
    // printf("Page %d next use is %d away\n",frames[i],stepsToNext);
    // Means it is infinty, smallest infinity is the one replaced
    if (stepsToNext == -1) {
      return i;
    }
    // If i == 0, set that as min
    if (i == 0) {
      furthestNum = stepsToNext;
    }
    if (stepsToNext > furthestNum) {
      furthestIndex = i;
      furthestNum = stepsToNext;
    }
  }
  return furthestIndex;
}
