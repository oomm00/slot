# Slot Machine Casino Management System

A full-stack casino slot machine application with advanced analytics, betting strategies, and fraud detection built using C++ backend and React frontend.

## Features

### Core Gameplay
- **Slot Machine Engine**: 3-reel weighted random number generator with configurable symbol probabilities
- **Multiple Bet Types**: 
  - Exact Prediction (50x payout)
  - Triple Symbol (30x payout)
  - Pair Prediction (8x payout)
  - Symbol Appearance (3x payout)
  - Any Pair (10x payout)
  - Any Triple (25x payout)
- **Real-time Balance Management**: Track player credits, wins, losses, and betting history

### Advanced Analytics
- **Betting Strategy Advisor**: 
  - Personalized recommendations based on betting history
  - Risk profile analysis (streak detection, balance trends)
  - Gambler's ruin probability calculations using Dynamic Programming
  - Real-time EV (Expected Value) computations
- **Performance Dashboard**: System-wide analytics including house edge, win rates, and player statistics
- **Strategy Lab**: Compare Greedy, Dynamic Programming, and Kelly Criterion betting strategies
- **Distribution Analysis**: Visualize balance evolution and bust probabilities over multiple rounds

### Fraud Detection
- **Real-time Fraud Scoring**: Detect suspicious betting patterns
- **Statistical Analysis**: Z-score anomaly detection and Rabin-Karp pattern matching
- **Player Flagging**: Automatic alerts for high-risk behaviors

### Player Management
- **User Authentication**: Secure login/registration system
- **Player Profiles**: Track individual player statistics and history
- **Leaderboards**: Rankings by balance, win rate, total winnings, and more
- **Search Engine**: Fast player, game, and bet lookup with case-insensitive search

## Technology Stack

### Backend (C++)
- **HTTP Server**: cpp-httplib for REST API
- **Data Structures**: Custom implementations of heaps, hash tables, graphs, and order-statistic trees
- **Algorithms**:
  - Dynamic Programming (betting strategies, probability calculations)
  - Binary Search (game/bet lookups)
  - String Matching (KMP, Rabin-Karp for fraud detection)
  - Graph Algorithms (Markov chains for probability analysis)
- **Data Storage**: CSV-based persistence for players, games, bets, and reports

### Frontend (React + Vite)
- **UI Framework**: React 19 with React Router for navigation
- **Styling**: Tailwind CSS 4 for modern, responsive design
- **HTTP Client**: Axios for API communication
- **State Management**: React Context API for global state

## Project Structure

```
slot/
├── src/
│   ├── algorithms/          # Core data structures (Heap, HashTable, OST, etc.)
│   ├── analytics/
│   │   ├── BettingAdvisor/  # Personalized betting recommendations
│   │   ├── BettingEngine/   # Bet processing and Kelly Criterion
│   │   ├── FraudDetector/   # Anomaly detection algorithms
│   │   ├── Leaderboard/     # Player rankings using Order-Statistic Trees
│   │   ├── ProbabilityAnalyzer/ # Markov chains and DP probability
│   │   └── SearchEngine/    # Binary search and string matching
│   ├── api/                 # REST API server (ApiServer.cpp)
│   ├── core/                # Slot machine engine (RNG, reels, paylines)
│   ├── managers/            # Player management
│   ├── models/              # Data models (Player, Bet, GameRecord)
│   ├── storage/             # CSV persistence layer
│   └── ui/                  # (Optional) Qt UI components
├── data/                    # CSV data files (players, games, bets, reports)
├── docs/                    # Documentation
├── tests/                   # Unit and integration tests
└── include/                 # Third-party headers (httplib, json)

slot-ui/
├── src/
│   ├── components/          # Reusable UI components
│   ├── context/             # React Context (AppContext)
│   ├── pages/               # Main pages (Dashboard, Betting, Analytics, etc.)
│   ├── services/            # API client services
│   └── utils/               # Utility functions
└── public/                  # Static assets
```

## Installation & Setup

### Prerequisites
- **C++ Compiler**: g++ with C++20 support (MinGW-w64 on Windows or GCC on Linux/Mac)
- **Node.js**: v18+ for frontend development
- **npm**: Package manager (comes with Node.js)

### Backend Setup

1. **Build the server** (if not already compiled):
   ```bash
   cd slot
   # If using g++ directly:
   g++ -std=c++20 -O2 -o server.exe \
     src/main.cpp \
     src/**/*.cpp \
     -Iinclude -Isrc/*/
   ```

2. **Run the backend server**:
   ```bash
   ./server.exe
   ```
   Server will start on `http://localhost:8080`

### Frontend Setup

1. **Navigate to frontend directory**:
   ```bash
   cd slot-ui
   ```

2. **Install dependencies**:
   ```bash
   npm install
   ```

3. **Start development server**:
   ```bash
   npm run dev
   ```
   Frontend will start on `http://localhost:5173`

## Usage

### Access the Application
- If using `run.bat`: Open `http://localhost:8080`
- If frontend separately: Open `http://localhost:5173`

### Test Accounts
The system comes with pre-populated test data:

**Regular Users** (password: `password`):
- alice, bob, charlie, diana, eve, frank, grace, henry, iris, jack

**Admin Account**:
- Username: `admin`
- Password: `admin123`

### Getting Started

1. **Login with Test Account**:
   - Use any test account above (e.g., username: `alice`, password: `password`)
   - Or click "Register" to create a new account

2. **Register a New Account**:
   - Click "Sign Out" (if shown) to go to login page
   - Click "Register" tab
   - Fill in name, username, password, and age
   - Start with 1,000 credits

2. **Play the Slot Machine**:
   - Go to "Game" page
   - Select bet type (e.g., "Any Pair")
   - Enter bet amount
   - Make predictions if required (e.g., for Exact Prediction)
   - Click "SPIN"
   - Watch the advisor sidebar for smart betting recommendations!

3. **View Analytics**:
   - Navigate to "Betting Analyzer & Strategy Lab"
   - Adjust balance, bet amount, and rounds
   - Click "Analyze" to see:
     - Dashboard with system stats
     - Betting advisor recommendations
     - Strategy comparisons (Greedy vs DP vs Kelly)
     - Full bet type rankings
     - Distribution charts

4. **Check Leaderboards**:
   - Go to "Leaderboard" page
   - View top players by balance, win rate, or total winnings
   - Filter by different metrics

5. **Search**:
   - Use the "Search" page to find players, games, or bets
   - Search is case-insensitive (after server recompile)

## Data Files

Player, game, and bet data are stored in CSV files located in:
- `slot/data/players.csv` - Player accounts and statistics
- `slot/data/games.csv` - Game history
- `slot/data/bets.csv` - Bet records
- `slot/data/reports.csv` - Strategy reports

These files are automatically created and updated by the backend server.

## API Endpoints

### Authentication
- `POST /api/register` - Register new player
- `POST /api/login` - Login and get session token

### Players
- `GET /api/players` - Get all players (auth required)
- `GET /api/players/:id` - Get player by ID
- `PUT /api/players/:id/deposit` - Deposit credits
- `PUT /api/players/:id/withdraw` - Withdraw credits

### Gameplay
- `POST /api/spin` - Place bet and spin reels
- `GET /api/bet-types` - Get available bet types and payouts

### Analytics
- `GET /api/analytics/system` - System-wide statistics
- `GET /api/analytics/player/:id` - Player analytics
- `GET /api/analytics/distribution` - Balance distribution analysis
- `GET /api/analytics/compare` - Compare all bet types
- `GET /api/analytics/strategies` - Compare betting strategies
- `POST /api/betting/advise` - Get personalized betting advice

### Leaderboard
- `GET /api/leaderboard?n=10` - Get top N players
- `GET /api/leaderboard/top/:metric?n=10` - Get top by metric (balance, winrate, etc.)

### Fraud Detection
- `GET /api/fraud/player/:id` - Get player fraud score
- `GET /api/fraud/alerts` - Get fraud statistics

### Search
- `GET /api/search/player?q=name` - Search players by name (case-insensitive)
- `GET /api/search/game/:id` - Search game by ID
- `GET /api/search/bets/:id` - Search bet by ID
- `GET /api/search/date?start=...&end=...` - Search games by date range

## Key Algorithms & Data Structures

### Dynamic Programming
- **Betting Strategy Optimization**: Maximize EV while minimizing risk
- **Gambler's Ruin Simulation**: Calculate bust probabilities over N rounds
- **Probability Distribution**: Expected balance and variance calculations

### Data Structures
- **Order-Statistic Tree**: O(log n) leaderboard queries (rank, k-th player)
- **Hash Table**: O(1) player/game lookups
- **Min/Max Heap**: Top-K queries for analytics
- **Graph (Markov Chain)**: Model state transitions in gambling

### Search & Pattern Matching
- **Binary Search**: Fast game/bet lookups in sorted indices
- **KMP Algorithm**: Pattern detection in betting sequences
- **Rabin-Karp**: Rolling hash for fraud detection

### Probability & Statistics
- **Weighted RNG**: Binary search on CDF for symbol selection
- **Z-Score Anomaly Detection**: Statistical fraud detection
- **Kelly Criterion**: Optimal bet sizing
- **Markov Chains**: Win/loss probability modeling

## Betting Advisor

The **Betting Advisor** is a key feature that provides personalized recommendations:

### How It Works
1. **Compute Win Probabilities**: Uses combinatorics and reel weights to calculate exact p_win for each bet type
2. **Risk Profiling**: Analyzes player's recent history (last 20 bets) to compute:
   - Streak (consecutive wins/losses)
   - Balance trend (linear regression)
   - Recent win rate
   - Lambda (risk-aversion weight)
3. **Gambler's Ruin DP**: Simulates future rounds to estimate bust probability
4. **Scoring**: Ranks bets by `score = EV - lambda × bustProbability`
5. **Dynamic Reason**: Generates recommendation text from actual numeric values

### Features
- Real-time updates based on player state
- Different players with same bet amount get different recommendations
- Reason text changes based on streak and balance trend
- Shows full comparison table (not just winner)
- DP computation runs in <50ms for instant results

## Testing

Run backend tests:
```bash
cd slot/tests
./test_slotmachine.exe
./test_bettingengine.exe
./test_frauddetector.exe
./test_leaderboard.exe
./test_searchengine.exe
./test_systemintegration.exe
```

## Development

### Frontend Hot Reload
The Vite dev server supports hot module replacement - changes to React components update instantly.

### Backend Recompilation
After modifying C++ files, recompile the server and restart:
```bash
cd slot
# Recompile (depends on your build system)
g++ -std=c++20 -O2 -o server.exe src/**/*.cpp -Iinclude
./server.exe
```

## Known Limitations

### Betting Advisor Endpoint Not Available
The betting advisor feature (`POST /api/betting/advise`) exists in the source code but is **not available in the compiled executable** (`slot-server.exe`). The executable was built from an earlier version of the code before the advisor endpoint was added.

**Impact:**
- Betting advisor sidebar in Game page will not show recommendations
- Advisor tab in Analytics page will be empty

**To Fix:**
The project needs to be recompiled with a C++20-compatible toolchain:
```bash
# Example with g++
cd slot
g++ -std=c++20 -O2 -pthread -o slot-server.exe src/**/*.cpp -Iinclude -Isrc
```

**Workaround:**
The backend includes other analytics features that work:
- Strategy Lab (`/api/analytics/strategies`)
- Risk Analysis (`/api/analytics/risk`)
- Distribution Analysis (`/api/analytics/distribution`)
- Recommendation Engine (`/api/analytics/recommend`)

## Dataset

The system includes pre-populated test data in `slot/data/`:

### Players (11 accounts)
- **10 regular players** (P1-P10): alice, bob, charlie, diana, eve, frank, grace, henry, iris, jack
- **1 admin** (P11): admin
- All regular users share password: `password`
- Admin password: `admin123`

### Game History (50 games)
- Realistic play sessions across June 10-12, 2024
- Mix of wins and losses showing various bet types
- Players at different skill/risk levels (conservative to aggressive)
- Includes big wins (up to 3000), losing streaks, and steady grinders

### Bets (50 records)
- Coordinated with game history
- Different bet types: ANY_PAIR, ANY_TRIPLE, SYMBOL_APPEARANCE, PAIR_PREDICTION, TRIPLE_SYMBOL, EXACT_PREDICTION
- Risk scores and expected values calculated
- Includes predictions for specific symbol bets

### Strategy Reports (28 reports)
- Historical betting advisor recommendations
- Shows DP/Greedy/Kelly strategy suggestions
- Risk profiles and confidence scores
- Demonstrates how recommendations change based on player history

## Troubleshooting

### "401 Unauthorized" errors
- Make sure you're logged in
- Check that your session token is valid
- Try logging out and back in

### Analytics page not loading
- Ensure backend server is running on port 8080
- Check browser console for errors
- Verify you're authenticated

### Betting advisor not showing
- Set a bet amount first
- Check that you have a playerId in your session
- Click "Refresh" button to manually fetch

### Backend won't start
- Check if port 8080 is already in use
- Verify all CSV data files exist in `slot/data/`
- Check server logs for error messages

## Credits

**Course**: Design and Analysis of Algorithms (DAA)  
**Built with**: C++20, React 19, Tailwind CSS 4, cpp-httplib, Vite 8

## License

This project is for educational purposes as part of a university course.
