# Cash Flow Minimzer System

24K-2523   Shuzain Ali
24K-2558.  Aziz Ur Rehman

## Introduction
The Cash Flow Minimizer System is a data structure–based application that helps a group of individuals, departments, or organizations settle mutual debts with the least number of transactions possible. In many real-world situations, multiple people owe each other varying amounts, and performing all transactions separately becomes inefficient.
This system automatically computes the optimized set of transactions required to settle all debts, minimizing total cash flow and making the settlement process faster and simpler. The project will include an interactive Qt-based GUI where users can input debts, view results, and visualize minimized transactions.

## Description
•	Users will input the number of participants and the amount each person owes to others.

•	The system will compute each person’s net balance (amount to pay or receive).

•	Using a greedy algorithm, the program will repeatedly match the highest debtor and highest creditor to minimize the number of transactions.

•	Results will be displayed in a clear, readable list (e.g., “A pays ₹100 to C”).

•	A graphical visualization (using Qt’s QGraphicsView) will show participants as nodes and optimized payments as directional edges.

•	The GUI will also allow users to save or load transaction data, reset inputs, and optionally export results.

•	This project combines both algorithmic logic and user-friendly design, making it a complete data structure application with practical real-world use.



## Data Structures that will be used
| **Data Structure**                      | **Where Used**                                   | **Purpose / Function**                            |
| :-------------------------------------: | :----------------------------------------------: | :-----------------------------------------------: |
| **Arrays / Vectors**                    | To store net balances and user names             | Maintain dynamic lists of people and balances     |
| **2D Matrix (Adjacency Matrix)**        | To store the original debts between participants | Represent graph of payments between all users     |
| **Priority Queue (optional)**           | To quickly find max debtor and creditor          | Optimize selection during cash flow minimization  |
| **Classes / Objects**                   | For `Person` and `CashFlow` system               | Encapsulate data and logic in OOP structure       |
| **File Handling (Text/CSV)**            | To save and load previous debt data              | Persistent storage of user sessions               |
| **Graph Structure (via Visualization)** | In GUI (QGraphicsScene)                          | Represent flow of minimized transactions visually |


