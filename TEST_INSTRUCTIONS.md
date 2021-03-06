## cloning and building the project.

    you can clone the project with the from the link:

    next build gem5 with:

        scons build/X86/gem5.opt -j [CPU NUM]

## Testing 

We have provided 2 testing scripts we used for our evaluation, due to issues of having particular sets of tests working on one system and failing on another. We have included 2 scripts each used by a group member to evaluate the implementation.

The first script is "tournament_test1.sh", which we used to evalate the performance of our predictor compared to other predictors.

In the config file you will find several options to set the cache sizes, cpu model, and branch predictor.

You also need to specify gem5 binary path, stat dumping directory, and benchmark directory.

Once the options are set the tests can be done by using the commands
        
        Make the script execuatable:

             chmod +x tournament_test1.sh 

        Run the script:

            ./tournament_test1.sh


The second script is "tournament_test2.sh", which we used to evaluate the perfomrmance of our predictor for various budget bits. Similar to test 1 

        Make the script execuatable:

            chmod +x tournament_test2.sh 

        Run the script:

            ./tournament_test2.sh

## Implementation Disclosures

Due to original implementation of the Multiperspective Perceptron Predictor for different sizes(8KB, 64KB), we had to make 2 versions of our predictor one with 8KB MPP and another with 64KB MPP. the predictors have object names of Tournament2BP and Tournament3BP, respectively. 

To modify the TAGE configuration:

1. Open the file /src/cpu/pred/BranchPredictor.py

2. Go to the TAGEBase Predictor Object

and Modify the lines below as:

For TAGE 1 configuration: 

    nHistoryTables = Param.Unsigned(4, "Number of history tables")
    minHist = Param.Unsigned(4, "Minimum history size of TAGE")
    maxHist = Param.Unsigned(16, "Maximum history size of TAGE")

    tagTableTagWidths = VectorParam.Unsigned(
        [0, 7, 7, 8, 8], "Tag size in TAGE tag tables")
    logTagTableSizes = VectorParam.Int(
        [10, 10, 10, 11, 11], "Log2 of TAGE table sizes")

For TAGE 2 configuration:

    nHistoryTables = Param.Unsigned(9, "Number of history tables")
    minHist = Param.Unsigned(4, "Minimum history size of TAGE")
    maxHist = Param.Unsigned(160, "Maximum history size of TAGE")

    tagTableTagWidths = VectorParam.Unsigned(
        [0, 7, 7, 8, 8, 9, 10, 11, 12, 12], "Tag size in TAGE tag tables")
    logTagTableSizes = VectorParam.Int(
        [10, 10, 10, 11, 11, 11, 11, 10, 10, 10], "Log2 of TAGE table sizes")

For TAGE 3 configuration:

    nHistoryTables = Param.Unsigned(12, "Number of history tables")
    minHist = Param.Unsigned(4, "Minimum history size of TAGE")
    maxHist = Param.Unsigned(640, "Maximum history size of TAGE")

    tagTableTagWidths = VectorParam.Unsigned(
        [0, 7, 7, 8, 8, 9, 10, 11, 12, 12, 13, 14, 15], "Tag size in TAGE tag tables")
    logTagTableSizes = VectorParam.Int(
        [10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9], "Log2 of TAGE table sizes")

You can also modify the Choice Table predictor size, by going to the Tournament2BP and Tournament2BP objects in BranchPredictor.py and modifying the parameter

After the modifications you can re-build gem5 as:

    scons build/X86/gem5.opt -j [CPU NUM]

And run the tests as shown above in the section "Testing".