
# Running the project-
    Compile-
        make
    Run-
        ./a3 <numOfCPUs> <S> <fileName>


    Custom workload file  = "customtasks.txt"



# Sample test results- 
    S = 100
    | Type | avg. turnaround(usecs) | avg. response time(usecs) | numOfCPUs |
    | 0    | 375155                 | 21715                     | 1         |
    | 1    | 345446                 | 19626                     | 1         |
    | 2    | 551867                 | 437925                    | 1         |
    | 3    | 437925                 | 22165                     | 1         |
    | 0    | 165210                 | 4385                      | 2         |
    | 1    | 410974                 | 4094                      | 2         |
    | 2    | 396583                 | 3540                      | 2         |
    | 3    | 328715                 | 5681                      | 2         |
    | 0    | 73169                  | 1482                      | 4         |
    | 1    | 175893                 | 1481                      | 4         |
    | 2    | 443390                 | 1185                      | 4         |
    | 3    | 338909                 | 1607                      | 4         |
    | 0    | 32960                  | 647                       | 8         |
    | 1    | 77810                  | 668                       | 8         |
    | 2    | 224827                 | 493                       | 8         |
    | 3    | 187064                 | 717                       | 8         |

    S = 500
    | Type | avg. turnaround(usecs) | avg. response time(usecs) | numOfCPUs |
    | 0    | 345309                 | 15005                     | 1         |
    | 1    | 416005                 | 12725                     | 1         |
    | 2    | 590205                 | 11608                     | 1         |
    | 3    | 476777                 | 17727                     | 1         |
    | 0    | 178871                 | 5730                      | 2         |
    | 1    | 423481                 | 5559                      | 2         |
    | 2    | 385755                 | 4537                      | 2         |
    | 3    | 323642                 | 6822                      | 2         |
    | 0    | 82048                  | 1462                      | 4         |
    | 1    | 183449                 | 1520                      | 4         |
    | 2    | 439914                 | 1162                      | 4         |
    | 3    | 332528                 | 1560                      | 4         |
    | 0    | 34093                  | 580                       | 8         |
    | 1    | 78343                  | 593                       | 8         |
    | 2    | 223183                 | 425                       | 8         |
    | 3    | 184970                 | 657                       | 8         |


    S = 1000

    | Type | avg. turnaround(usecs) | avg. response time(usecs) | numOfCPUs |
    | 0    | 338622                 | 17306                     | 1         |
    | 1    | 448825                 | 14993                     | 1         |
    | 2    | 521886                 | 12969                     | 1         |
    | 3    | 496787                 | 20610                     | 1         |
    | 0    | 174714                 | 4945                      | 2         |
    | 1    | 428676                 | 4713                      | 2         |
    | 2    | 378448                 | 3980                      | 2         |
    | 3    | 351633                 | 6094                      | 2         |
    | 0    | 78178                  | 1463                      | 4         |
    | 1    | 180918                 | 1493                      | 4         |
    | 2    | 443205                 | 1204                      | 4         |
    | 3    | 335432                 | 1535                      | 4         |
    | 0    | 33252                  | 635                       | 8         |
    | 1    | 78313                  | 695                       | 8         |
    | 2    | 224360                 | 486                       | 8         |
    | 3    | 185027                 | 643                       | 8         |

    S = 2000

    | Type | avg. turnaround(usecs) | avg. response time(usecs) | numOfCPUs |
    | 0    | 307877                 | 17404                     | 1         |
    | 1    | 425257                 | 14641                     | 1         |
    | 2    | 523019                 | 13186                     | 1         |
    | 3    | 492538                 | 20875                     | 1         |
    | 0    | 190617                 | 4870                      | 2         |
    | 1    | 440267                 | 4679                      | 2         |
    | 2    | 408005                 | 3983                      | 2         |
    | 3    | 361618                 | 6174                      | 2         |
    | 0    | 74067                  | 1463                      | 4         |
    | 1    | 174045                 | 1493                      | 4         |
    | 2    | 443205                 | 1204                      | 4         |
    | 3    | 335432                 | 1535                      | 4         |
    | 0    | 74067                  | 1533                      | 8         |
    | 1    | 174045                 | 1614                      | 8         |
    | 2    | 437575                 | 1277                      | 8         |
    | 3    | 329949                 | 1541                      | 8         |
