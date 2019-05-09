# CrowdPareto

## Overview:
This repository includes the supplementary materials for our CIKM 2015 paper: "Crowdsourcing Pareto-Optimal Object Finding By Pairwise Comparisons".
Please use the bib file "asudeh2015crowdsourcing.bib" and cite the paper if you use the materials.

1- The paper itself is included in "Paper/crowdpareto-cikm15.pdf"

2- The CPP code for the algorithms is in "Code/ParetoWithCrowd.cpp"

3- The (main) datasets are included in the folder "Datasets"

## Real Experiment Using Amazon Mechanical Turk. Data in "Datasets/AMT"
In one of the experiments, we studied the performance of the proposed algorithms using the popular crowdsourcing marketplace Amazon Mechanical Turk (AMT).
The task is to compare 100 photos (located in "Datasets/AMT/Pictures") of the University of Texas at Arlington with regard to three criteria: <b> color, sharpness, and landscape </b>. To obtain the ground-truth data, all 14,850 possible pairwise questions were partitioned into 1, 650 tasks, each containing 9 questions on a criterion.
An AMT crowdsourcer is allowed to perform a task only if they have responded to at least 100 HITs (Human Intelligence Tasks)
before with at least 90% approval rate.


