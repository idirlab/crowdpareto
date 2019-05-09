# CrowdPareto

## Overview:
This repository includes the supplementary materials for our CIKM 2015 paper: "Crowdsourcing Pareto-Optimal Object Finding By Pairwise Comparisons".
Please use the bib file "asudeh2015crowdsourcing.bib" and cite the paper if you use the materials.

1- The paper itself is included in "Paper/crowdpareto-cikm15.pdf"

2- The CPP code for the algorithms is in "Code/ParetoWithCrowd.cpp"

3- The (main) datasets are included in the folder "Datasets"

## Real Experiment Using Amazon Mechanical Turk
### Location: "Datasets/AMT"
In one of the experiments, we studied the performance of the proposed algorithms using the popular crowdsourcing marketplace Amazon Mechanical Turk (AMT).
The task is to compare 100 photos (located in "Datasets/AMT/Pictures") of the University of Texas at Arlington with regard to three criteria: *color, sharpness, and landscape*. To obtain the ground-truth data, all 14,850 possible pairwise questions were partitioned into 1, 650 tasks, each containing 9 questions on a criterion.
An AMT crowdsourcer is allowed to perform a task only if they have responded to at least 100 HITs (Human Intelligence Tasks)
before with at least 90% approval rate.
Furthermore, we implemented basic quality control by including 2 additional validation questions in each task that expect certain answers. For instance, one such question asks the crowd to compare a colorful photo and a dull photo by criterion color. A crowdsourcer’s responses in a task are discarded if their response to a validation question deviates from our expectation.
We paid 1 cent for each comparison and therefore spent close to $1,000 in total.

The raw collection of all crow responses is located in the file: "Datasets/AMT/Raw_AMT_results.txt"
Every line in the file has the format x;y;z;R
- x: the dimension (criterion) index --  (0) color, (1) sharpness, (2) landscape
- y: the index of the first picture
- z: the index of the sexond item
- R: the (comma-seprated) vector of crows responses. In every element of the vector: (1) means that the user prefers y over z on criterion x, (-1) z is preferred over y on criterion x, (0) y and z are incomparable (indifferent) on criterion x
