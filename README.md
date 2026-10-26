# Smart Wardrobe

A simple web-based outfit recommendation system built to make everyday outfit selection faster and easier.

The user enters their wardrobe once, including the type, subtype, color, occasion, and season suitability of each clothing item. The system then generates a small set of outfit combinations using predefined compatibility rules based on clothing type, subtype, color, occasion, and season.

The primary purpose of the project is to demonstrate **Object-Oriented Programming in C++ through a practical application**, while also providing a usable web interface.

---

## 1. Project Overview

Choosing an outfit every day is a small problem, but it is repeated frequently and can waste time and mental energy.

**Smart Wardrobe** attempts to solve this problem by turning a user's existing wardrobe into a structured collection of clothing objects and automatically generating suitable outfit combinations.

Instead of asking the user to decide:

> "Which shirt goes with these jeans?"

the application evaluates the available clothing and produces combinations that satisfy a set of compatibility rules.

For example:

* A plain T-shirt may be compatible with jeans and sneakers.
* A formal shirt may be compatible with trousers and formal shoes.
* A shirt may be marked as casual, but still be considered incompatible with sweatpants.
* Two individually valid clothing items may still produce a poor outfit when combined.

The system therefore considers **relationships between clothing items**, rather than treating every item as interchangeable.

---

# 2. Problem Statement

People often own enough clothes but still spend unnecessary time deciding what to wear.

The problem is not necessarily a lack of clothing. It is the difficulty of quickly identifying a combination that satisfies several constraints simultaneously:

* Occasion
* Season
* Color compatibility
* Clothing type
* Clothing subtype
* Compatibility between individual garments

Smart Wardrobe provides a rule-based system that handles these decisions automatically.

---

# 3. Objectives

The project has five main objectives:

1. Create a structured digital representation of a user's wardrobe.
2. Use C++ Object-Oriented Programming to model different categories of clothing.
3. Generate outfits using practical compatibility rules.
4. Provide a simple web interface so the system is easy to use.
5. Store wardrobe data persistently using CSV files without requiring a database for the initial version.

The project deliberately avoids unnecessary complexity such as machine learning, image recognition, user authentication, or large-scale recommendation infrastructure.

---

# 4. Core User Workflow

The intended workflow is:

```text
User opens Smart Wardrobe
        ↓
Adds clothing items
        ↓
Selects:
    Type
    Subtype
    Color
    Occasion
    Season
        ↓
Wardrobe is stored
        ↓
User selects desired occasion/season
        ↓
Recommendation Engine examines wardrobe
        ↓
Invalid combinations are removed
        ↓
Remaining outfits are scored
        ↓
Top few outfits are returned
        ↓
Frontend displays outfit cards
```

The user should not need to understand any of the underlying OOP logic.

---

# 5. Main Features

## 5.1 Wardrobe Management

The user can:

* Add clothing items
* Remove clothing items
* View all clothing items
* Search clothing
* Filter clothing by:

  * Type
  * Subtype
  * Color
  * Occasion
  * Season
* Edit existing clothing information

---

## 5.2 Clothing Classification

Each item belongs to one of three major categories:

```text
Top
Bottom
Shoes
```

Examples:

### Tops

* T-Shirt
* Shirt
* Hoodie
* Sweater

### Bottoms

* Jeans
* Cargo
* Trousers
* Sweatpants

### Shoes

* Sneakers
* Boots
* Formal Shoes

The exact list should remain controlled rather than allowing arbitrary user-created categories.

This gives the recommendation engine predictable inputs.

---

## 5.3 Occasion Tags

Clothing can be associated with one or more occasions.

Initial controlled options:

```text
Casual
Smart Casual
Formal
Active
Party
```

The system should treat these as **tags**, not subclasses.

For example:

```text
Shirt
    → Casual
    → Smart Casual
    → Formal
```

A formal shirt is still fundamentally a `Shirt`.
"Formal" describes where or when it is appropriate.

---

## 5.4 Season Tags

Each item can also have season suitability.

Initial options:

```text
All Season
Summer
Winter
Monsoon
```

This allows the recommendation engine to avoid combinations that make little practical sense.

For example:

```text
Summer T-Shirt + Summer Jeans + Sneakers
```

may receive a strong match for a summer casual request.

A heavy winter sweater may receive a low score or be rejected for the same request.

---

## 5.5 Outfit Recommendation

The core feature of the application is outfit generation.

An outfit consists of:

```text
1 Top
+
1 Bottom
+
1 Pair of Shoes
```

The system generates several possible combinations rather than returning only one.

For example:

```text
Outfit 1
White T-Shirt
Blue Jeans
White Sneakers

Outfit 2
Black T-Shirt
Grey Cargos
Black Sneakers

Outfit 3
Blue Shirt
Black Trousers
Black Formal Shoes
```

The combinations are ranked using compatibility rules.

---

## 5.6 Color Compatibility

The system should not attempt complicated fashion analysis.

Instead, colors should be grouped into simple categories and evaluated using predefined rules.

For example:

```text
Neutral:
Black
White
Grey
Beige
Brown

Blue family:
Light Blue
Blue
Navy

Other:
Red
Green
etc.
```

The recommendation engine can then use a small compatibility table.

Example:

```text
White + Blue       → Strong
Black + Grey       → Strong
White + Black      → Strong
Blue + Black       → Strong
Red + Green        → Weak
```

The exact rules can be expanded later, but the initial implementation should remain deterministic.

---

# 6. Recommendation Logic

Smart Wardrobe is **rule-based, not machine-learning based**.

This is intentional.

The goal of the project is to demonstrate OOP and build a functioning recommendation system, not to create a production-scale AI fashion model.

The recommendation engine follows several stages.

## Stage 1 — Category Validation

Every outfit must contain:

```text
1 Top
1 Bottom
1 Shoe
```

The system must never produce:

```text
Top + Top + Shoes
```

or:

```text
Bottom + Bottom + Shoes
```

---

## Stage 2 — Occasion Compatibility

The selected occasion is compared with the tags attached to each clothing item.

Example:

```text
Requested occasion:
Casual
```

Potentially valid:

```text
T-Shirt → Casual
Jeans → Casual
Sneakers → Casual
```

Potentially invalid:

```text
Formal Shirt → Formal only
Formal Shoes → Formal only
```

An item that supports multiple occasions can be used in multiple contexts.

---

## Stage 3 — Season Compatibility

The selected season is compared against the item's season tags.

Example:

```text
Requested season:
Winter

Sweater → Winter
Jeans → All Season
Boots → Winter
```

This combination is valid.

---

## Stage 4 — Subtype Compatibility

This is one of the most important rules.

Two items can individually satisfy the occasion but still make a poor combination.

For example:

```text
Shirt + Sweatpants
```

may technically satisfy a broad "Casual" tag but should receive a very low compatibility score or be rejected.

The system therefore maintains a predefined subtype compatibility matrix.

Example:

| Top     | Bottom     | Compatibility |
| ------- | ---------- | ------------- |
| T-Shirt | Jeans      | Strong        |
| T-Shirt | Cargo      | Strong        |
| T-Shirt | Sweatpants | Strong        |
| Shirt   | Trousers   | Strong        |
| Shirt   | Jeans      | Medium/Strong |
| Shirt   | Cargo      | Medium        |
| Shirt   | Sweatpants | Weak          |
| Hoodie  | Jeans      | Strong        |
| Hoodie  | Cargo      | Strong        |
| Hoodie  | Sweatpants | Strong        |
| Sweater | Jeans      | Strong        |
| Sweater | Trousers   | Strong        |

This matrix should remain small and understandable.

---

## Stage 5 — Shoe Compatibility

Shoes are evaluated against the overall outfit.

Examples:

```text
T-Shirt + Jeans + Sneakers
→ Strong

Shirt + Trousers + Formal Shoes
→ Strong

Hoodie + Sweatpants + Formal Shoes
→ Weak
```

---

## Stage 6 — Color Score

Once basic compatibility has been established, color compatibility contributes to the overall score.

---

## Stage 7 — Final Outfit Score

Each valid outfit receives a score.

A simplified conceptual model:

```text
Outfit Score =
    Occasion Compatibility
  + Season Compatibility
  + Top/Bottom Compatibility
  + Shoe Compatibility
  + Color Compatibility
```

The exact numerical weights are implementation details and can be tuned during development.

The system returns the top few valid combinations.

---

# 7. Example

## Input

The user has entered:

```text
Item 1
Type: Top
Subtype: T-Shirt
Color: White
Occasion: Casual
Season: All Season

Item 2
Type: Top
Subtype: Shirt
Color: Blue
Occasion: Formal, Smart Casual
Season: All Season

Item 3
Type: Bottom
Subtype: Jeans
Color: Blue
Occasion: Casual, Smart Casual
Season: All Season

Item 4
Type: Bottom
Subtype: Trousers
Color: Black
Occasion: Formal, Smart Casual
Season: All Season

Item 5
Type: Bottom
Subtype: Sweatpants
Color: Grey
Occasion: Casual, Active
Season: All Season

Item 6
Type: Shoes
Subtype: Sneakers
Color: White
Occasion: Casual, Smart Casual
Season: All Season

Item 7
Type: Shoes
Subtype: Formal Shoes
Color: Black
Occasion: Formal
Season: All Season
```

The user requests:

```text
Occasion: Casual
Season: All Season
```

## Possible Output

```text
Recommended Outfit 1

White T-Shirt
Blue Jeans
White Sneakers

Why:
- Casual occasion match
- All-season compatibility
- Strong T-Shirt + Jeans combination
- Strong color combination
```

```text
Recommended Outfit 2

White T-Shirt
Grey Sweatpants
White Sneakers

Why:
- Casual occasion match
- Strong subtype compatibility
- Compatible neutral colors
```

The formal shirt + sweatpants combination would be rejected even though the broad occasion tags may overlap.

---

# 8. System Architecture

The application uses a simple layered architecture.

```text
┌──────────────────────────────────────┐
│              FRONTEND                │
│        HTML + CSS + JavaScript       │
│                                      │
│  Forms / Wardrobe / Recommendations │
└──────────────────┬───────────────────┘
                   │ HTTP / JSON
                   ▼
┌──────────────────────────────────────┐
│           C++ BACKEND API            │
│        Native HTTP (Winsock)         │
│                                      │
│ API Routes / Request Handling        │
└──────────────────┬───────────────────┘
                   ▼
┌──────────────────────────────────────┐
│         APPLICATION LOGIC            │
│                                      │
│ WardrobeManager                      │
│ RecommendationEngine                 │
└──────────────────┬───────────────────┘
                   ▼
┌──────────────────────────────────────┐
│           DOMAIN MODEL               │
│                                      │
│ Clothing                             │
│ ├── Top                              │
│ │   ├── Shirt                        │
│ │   ├── T-Shirt                      │
│ │   ├── Hoodie                       │
│ │   └── Sweater                      │
│ │                                    │
│ ├── Bottom                           │
│ │   ├── Jeans                        │
│ │   ├── Cargo                        │
│ │   ├── Trousers                     │
│ │   └── Sweatpants                   │
│ │                                    │
│ └── Shoes                            │
│     ├── Sneakers                     │
│     ├── Boots                        │
│     └── Formal Shoes                 │
│                                      │
│ Outfit                               │
└──────────────────┬───────────────────┘
                   ▼
┌──────────────────────────────────────┐
│            DATA LAYER                │
│                                      │
│           CSVRepository              │
│                                      │
│         wardrobe.csv                 │
└──────────────────────────────────────┘
```

---

# 9. Technology Stack

The project should use a deliberately lightweight stack.

## Frontend

```text
HTML5
CSS3
Vanilla JavaScript
```

No React, Angular, Vue, or other frontend framework is necessary.

The interface is small enough that a framework would add complexity without providing meaningful academic value.

---

## Backend

```text
C++17
Native Winsock HTTP Server (Crow alternative)
```

The backend is implemented as a **native C++ HTTP/REST server using Winsock**, providing the HTTP server and REST API layer with zero external network dependencies, while the actual application logic remains strictly in C++ domain classes. (Lightweight frameworks like **Crow** represent a conceptual alternative, but the native socket implementation keeps the build self-contained without requiring Boost or Asio).

The important OOP work should not be buried inside the framework.

---

## Data Format

```text
CSV
```

Wardrobe data is initially stored in CSV files.

This keeps the system:

* Simple
* Human-readable
* Easy to inspect
* Easy to modify
* Appropriate for the project's scale

SQLite can be introduced later without changing the core domain classes if the CSV implementation becomes limiting.

---

## JSON

```text
JSON
```

JSON is used for communication between the browser and C++ backend.

`nlohmann/json` can be used for JSON serialization/deserialization.

---

## Build System

```text
CMake
```

CMake is preferred over manually compiling several `.cpp` files because the project contains multiple backend components and external C++ dependencies.

---

## Version Control

```text
Git
GitHub
```

---

# 10. Backend Architecture

The backend is divided into three practical sections.

## 10.1 API Layer

Responsible for:

* Receiving HTTP requests
* Validating request data
* Calling application logic
* Returning JSON responses

Example endpoints:

```text
GET    /api/clothes
POST   /api/clothes
PUT    /api/clothes/{id}
DELETE /api/clothes/{id}

GET    /api/outfits
POST   /api/outfits/generate

GET    /api/health
```

---

## 10.2 Application Layer

### WardrobeManager

Responsible for managing the wardrobe.

Typical responsibilities:

```text
addItem()
removeItem()
updateItem()
getAllItems()
findItems()
filterItems()
```

### RecommendationEngine

Responsible for:

```text
filtering candidates
checking compatibility
calculating outfit scores
generating outfit combinations
ranking results
```

The recommendation logic should be independent of the frontend.

---

# 11. Domain Model

The domain model is the main area where Object-Oriented Programming is demonstrated.

A simple inheritance hierarchy is:

```text
                    Clothing
                       │
            ┌──────────┼──────────┐
            │          │          │
           Top       Bottom      Shoes
            │          │          │
       ┌────┼────┐  ┌──┼────┐  ┌──┼──────────┐
       │    │    │  │  │    │  │  │          │
    Shirt TShirt Hoodie  Jeans Cargo Trousers Sneakers Boots
                      │
                 Sweatpants
```

A `Sweater` can also be added under `Top`.

The exact number of subclasses should remain small enough for the project to remain maintainable.

---

# 12. Clothing Base Class

The base `Clothing` class stores information shared by all clothing items.

Conceptually:

```text
Clothing
├── id
├── color
├── occasionTags
├── seasonTags
├── common functions
└── virtual functions
```

Possible functions:

```text
getId()
getColor()
getOccasions()
getSeasons()
getCategory()
getSubtype()
isSuitableForOccasion()
isSuitableForSeason()
```

The class should provide common behavior while allowing derived classes to provide specialized information.

---

# 13. Top, Bottom and Shoes

These act as intermediate abstract classes.

```text
Clothing
    ↓
Top
```

```text
Clothing
    ↓
Bottom
```

```text
Clothing
    ↓
Shoes
```

They provide logical grouping and allow the recommendation engine to work with categories.

For example:

```cpp
std::vector<std::shared_ptr<Top>>
std::vector<std::shared_ptr<Bottom>>
std::vector<std::shared_ptr<Shoes>>
```

This makes it easy to generate:

```text
Top × Bottom × Shoes
```

combinations.

---

# 14. OOP Concepts Demonstrated

The project should intentionally demonstrate the major OOP concepts taught in the course.

## Encapsulation

Data such as:

```text
color
occasion
season
id
```

should be kept inside the clothing objects and accessed through appropriate member functions.

---

## Inheritance

Examples:

```text
Top → Shirt
Top → T-Shirt
Top → Hoodie
Bottom → Jeans
Bottom → Cargo
Bottom → Trousers
Shoes → Sneakers
Shoes → Boots
```

---

## Abstraction

`Clothing`, `Top`, `Bottom`, and `Shoes` can provide abstract interfaces that hide implementation details from the recommendation engine.

---

## Polymorphism

A collection can contain different derived clothing objects while the program interacts with them through a common base interface.

Conceptually:

```cpp
std::vector<std::shared_ptr<Clothing>>
```

can contain:

```text
Shirt
TShirt
Hoodie
Jeans
Sneakers
Boots
```

Virtual functions allow the appropriate derived behavior to execute at runtime.

This provides a clear demonstration of **dynamic binding**.

---

## Constructors

Constructors initialize clothing objects with their attributes.

Example:

```text
Shirt(
    id,
    color,
    occasionTags,
    seasonTags
)
```

---

## Destructors

Destructors handle object cleanup.

Virtual destructors should be used in polymorphic base classes.

---

## Friend Function

A friend function can be used naturally for object display/serialization.

Example:

```cpp
friend std::ostream& operator<<(
    std::ostream& out,
    const Clothing& item
);
```

This provides a legitimate use of the `friend` concept instead of adding an artificial feature purely to demonstrate it.

---

## Static Members

A static member can maintain information shared across instances.

A practical example is an automatically generated clothing ID:

```text
Clothing::nextId
```

Each newly created item can receive a unique ID.

---

# 15. Outfit Class

An `Outfit` represents one recommended combination.

Conceptually:

```text
Outfit
├── Top
├── Bottom
├── Shoes
├── Score
└── Compatibility explanation
```

Example:

```text
Outfit
    Top: White T-Shirt
    Bottom: Blue Jeans
    Shoes: White Sneakers
    Score: 91
```

The explanation allows the frontend to display why the outfit was recommended.

---

# 16. CSV Data Storage

The initial version uses CSV.

Example:

```csv
id,type,subtype,color,occasion,season
1,Top,T-Shirt,White,Casual,All Season
2,Bottom,Jeans,Blue,Casual|Smart Casual,All Season
3,Shoes,Sneakers,White,Casual|Smart Casual,All Season
4,Top,Shirt,Blue,Formal|Smart Casual,All Season
```

The repository layer is responsible for converting:

```text
CSV row
    ↓
C++ object
```

and:

```text
C++ object
    ↓
CSV row
```

The rest of the backend should not need to know how CSV files work.

---

# 17. Future SQLite Compatibility

CSV is appropriate for the first version because the project is small.

However, the system should avoid tightly coupling the recommendation engine to CSV.

The architecture should allow:

```text
CSVRepository
```

to later be replaced by:

```text
SQLiteRepository
```

without rewriting:

```text
Clothing classes
WardrobeManager
RecommendationEngine
Frontend API
```

This can be achieved by keeping data-access functionality inside a repository interface or isolated repository class.

SQLite is therefore a **possible future implementation**, not a requirement for version 1.

---

# 18. Frontend

The frontend should be a simple web application.

## Main Screens

### Dashboard

Shows:

```text
Total wardrobe items
Favourite outfits
Recently added items
Quick recommendation button
```

---

### Add Clothing

A form containing controlled inputs:

```text
Type
Subtype
Color
Occasion
Season
```

Example:

```text
Type:
[ Top ▼ ]

Subtype:
[ T-Shirt ▼ ]

Color:
[ White ▼ ]

Occasion:
[ Casual ▼ ]

Season:
[ All Season ▼ ]

[ Add to Wardrobe ]
```

The controlled input system is deliberate.

Allowing completely arbitrary text such as:

```text
whiteish
whit
Off-white
cream white
snow
```

would make deterministic recommendation rules harder to implement.

---

### Wardrobe

Displays clothing items in cards or a table.

Example:

```text
White T-Shirt
Top • T-Shirt
Casual • All Season

[Edit] [Delete]
```

Filters can be provided for:

```text
Type
Subtype
Color
Occasion
Season
```

---

### Recommendation Page

The user selects:

```text
Occasion
Season
```

and presses:

```text
Generate Outfits
```

The frontend receives several recommendations from the C++ backend.

Each result is shown as an outfit card.

---

# 19. Input

The main user inputs are:

```text
Clothing Type
Clothing Subtype
Color
Occasion
Season
```

Recommendation inputs are:

```text
Requested Occasion
Requested Season
```

Optional future settings may include:

```text
Favourite
Laundry Status
Previously Worn
```

These should not interfere with the core recommendation engine.

---

# 20. Output

The primary output is a set of recommended outfits.

Example:

```text
OUTFIT 1

Top:
White T-Shirt

Bottom:
Blue Jeans

Shoes:
White Sneakers

Score:
92

Reason:
Strong casual compatibility,
all-season compatibility,
and strong color matching.
```

The system should return a small number of results, such as:

```text
3–5 outfits
```

rather than overwhelming the user with every mathematically possible combination.

---

# 21. Additional Lightweight Features

The project can include several useful features without significantly changing its architecture.

## Laundry Status

An item can have a simple state:

```text
Available
In Laundry
```

Items marked `In Laundry` should not be recommended.

---

## Favourite Outfits

The user can mark generated outfits as favourites.

---

## Usage Tracking

The application can track how often an item has been used.

Example:

```text
Blue Jeans
Used: 13 times
```

This enables simple statistics such as:

```text
Most worn clothing
Least worn clothing
Most frequently recommended items
```

This does not require machine learning.

---

## Search and Filtering

The user can quickly find:

```text
All blue clothing
All shirts
All casual clothing
All winter clothing
```

---

# 22. Weather Integration

Weather-based recommendations are a possible extension.

However, **the initial version should not depend on an external weather API**.

A weather API introduces:

* API keys
* Network dependencies
* Additional error handling
* External service failures

For the course project, manually selecting the season is sufficient.

A future implementation could replace:

```text
User selects season
```

with:

```text
Weather service
       ↓
Current temperature/weather
       ↓
Recommended season/category
```

This should remain outside the initial implementation.

---

# 23. Project Structure

A recommended project structure is:

```text
smart-wardrobe/
│
├── backend/
│   ├── include/
│   │   ├── Clothing.h
│   │   ├── Top.h
│   │   ├── Bottom.h
│   │   ├── Shoes.h
│   │   ├── Shirt.h
│   │   ├── TShirt.h
│   │   ├── Hoodie.h
│   │   ├── Sweater.h
│   │   ├── Jeans.h
│   │   ├── Cargo.h
│   │   ├── Trousers.h
│   │   ├── Sweatpants.h
│   │   ├── Sneakers.h
│   │   ├── Boots.h
│   │   ├── Outfit.h
│   │   ├── WardrobeManager.h
│   │   ├── RecommendationEngine.h
│   │   └── CSVRepository.h
│   │
│   ├── src/
│   │   ├── Clothing.cpp
│   │   ├── Top.cpp
│   │   ├── Bottom.cpp
│   │   ├── Shoes.cpp
│   │   ├── Shirt.cpp
│   │   ├── TShirt.cpp
│   │   ├── Hoodie.cpp
│   │   ├── Sweater.cpp
│   │   ├── Jeans.cpp
│   │   ├── Cargo.cpp
│   │   ├── Trousers.cpp
│   │   ├── Sweatpants.cpp
│   │   ├── Sneakers.cpp
│   │   ├── Boots.cpp
│   │   ├── Outfit.cpp
│   │   ├── WardrobeManager.cpp
│   │   ├── RecommendationEngine.cpp
│   │   ├── CSVRepository.cpp
│   │   └── main.cpp
│   │
│   ├── data/
│   │   └── wardrobe.csv
│   │
│   └── CMakeLists.txt
│
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── app.js
│
├── README.md
└── .gitignore
```

The project should remain approximately this size.

Do not create unnecessary microservices, authentication servers, Docker infrastructure, cloud deployment systems, or separate databases unless the project requirements later change.

---

# 24. API Contract

The frontend communicates with the backend using JSON.

## Add Clothing

```http
POST /api/clothes
```

Example request:

```json
{
  "type": "Top",
  "subtype": "T-Shirt",
  "color": "White",
  "occasions": ["Casual"],
  "seasons": ["All Season"]
}
```

---

## Get Wardrobe

```http
GET /api/clothes
```

---

## Delete Clothing

```http
DELETE /api/clothes/{id}
```

---

## Generate Outfit

```http
POST /api/outfits/generate
```

Example request:

```json
{
  "occasion": "Casual",
  "season": "Summer"
}
```

Example response:

```json
{
  "outfits": [
    {
      "top": "White T-Shirt",
      "bottom": "Blue Jeans",
      "shoes": "White Sneakers",
      "score": 92,
      "reason": "Strong casual and color compatibility."
    }
  ]
}
```


### Outfit Recommendation Mapping

| Frontend Request Field | C++ `RecommendationCriteria` Field | Notes |
| :--- | :--- | :--- |
| `occasion` | `criteria.occasion` | Case-insensitive match against item occasion tags |
| `season` | `criteria.season` | Case-insensitive match against item season tags (or "All Season") |

| Frontend Response Field | C++ `Outfit` Accessor | Description |
| :--- | :--- | :--- |
| `top` | `outfit.getTopDescription()` | Format: `"<Color> <Subtype>"` (e.g. `"White T-Shirt"`) |
| `bottom` | `outfit.getBottomDescription()` | Format: `"<Color> <Subtype>"` (e.g. `"Blue Jeans"`) |
| `shoes` | `outfit.getShoesDescription()` | Format: `"<Color> <Subtype>"` (e.g. `"White Sneakers"`) |
| `score` | `outfit.getScore()` | Bounded integer score `0` to `100` |
| `reason` | `outfit.getReasons()` | Array of human-readable rationale strings |

### Demo Wardrobe Dataset
Located at `backend/data/demo_wardrobe.csv`. Contains 11 curated items across Tops, Bottoms, and Shoes demonstrating casual, formal, and seasonal combinations. Can be loaded via `CSVRepository::load("backend/data/demo_wardrobe.csv", manager)` in tests and upcoming API server initializations.

---

# 25. Error Handling

The backend should validate user input before creating objects.

Examples of invalid input:

```text
Unknown clothing type
Unknown subtype
Invalid color
Missing required field
Invalid clothing combination
```

The backend should return meaningful HTTP status codes and JSON error messages.

Example:

```json
{
  "error": "Invalid clothing subtype for selected type."
}
```

The frontend should display these errors cleanly rather than exposing raw server errors.

---

# 26. Design Principles

The project should follow a few simple principles.

### Keep business logic out of the frontend

JavaScript should not independently determine whether:

```text
Shirt + Sweatpants
```

is valid.

That logic belongs in the C++ backend.

---

### Keep storage logic separate from business logic

`RecommendationEngine` should not directly open files.

Instead:

```text
CSVRepository
    ↓
WardrobeManager
    ↓
RecommendationEngine
```

---

### Avoid excessive abstraction

The project does not need:

```text
20 interfaces
10 design patterns
microservices
AI models
cloud infrastructure
complex databases
```

The architecture exists to keep the code organized, not to make the project look larger than it is.

---

# 27. Non-Goals

The following are explicitly outside the initial project scope:

```text
Machine learning
Computer vision
Clothing image recognition
Virtual try-on
Face/body analysis
Large-scale user accounts
Social networking
E-commerce
Complex weather prediction
Cloud architecture
Real-time collaborative wardrobes
```

These features may sound impressive but would distract from the actual OOP objective.

---

# 28. Testing Strategy

Testing should concentrate on the recommendation rules and core classes.

Important test cases include:

```text
T-Shirt + Jeans + Sneakers
→ valid

Shirt + Trousers + Formal Shoes
→ valid

Shirt + Sweatpants
→ weak/rejected

Wrong occasion
→ rejected or heavily penalized

Wrong season
→ rejected or penalized

Item in laundry
→ never recommended

Invalid category
→ rejected

Missing required input
→ API error
```

The recommendation engine should be testable independently from the web frontend.

---

# 29. Future Scope

Once the base system works, possible extensions include:

```text
Weather-based recommendations
Wear history
Better preference modelling
More detailed color rules
Outfit history
Calendar/occasion reminders
SQLite persistence
User-specific preferences
Clothing images
Mobile-friendly UI
```

These are extensions, not prerequisites for the core project.

---

# 30. Expected Final Demonstration

A complete demonstration should show the following flow:

```text
1. Open web application
2. Add several wardrobe items
3. View the wardrobe
4. Filter/search items
5. Select an occasion and season
6. Generate outfits
7. Show multiple recommended combinations
8. Explain why one combination is valid
9. Demonstrate that an obviously incompatible combination is rejected
10. Demonstrate that data persists after restarting the backend
```

The OOP concepts should be visible in the code even though the final user interacts through the browser.

---

# 31. AI Implementation Specification

This section is intentionally written so that the README can also be supplied to an AI coding assistant as a project-generation specification.

## Project Goal

Build a small full-stack web application called **Smart Wardrobe**.

The application's purpose is to recommend everyday outfits from a user's existing wardrobe.

The backend **must be implemented in C++**.

The application should be simple enough for a college OOP project and must prioritize clean demonstration of OOP concepts over feature count.

---

## Required Stack

Use:

```text
Frontend:
HTML5
CSS3
Vanilla JavaScript

Backend:
C++17
Native Winsock HTTP Server (Crow alternative)

JSON:
nlohmann/json

Persistence:
CSV

Build:
CMake

Version Control:
Git
```

Do not introduce a frontend framework unless absolutely necessary.

Do not introduce a database for the first version.

Do not introduce machine learning.

---

## Required Backend Architecture

Implement:

```text
HTTP/API Layer
        ↓
WardrobeManager
        ↓
Domain Objects
        ↓
CSVRepository
```

The recommendation process should use:

```text
RecommendationEngine
```

as a separate component.

---

## Required OOP Hierarchy

Implement an inheritance hierarchy similar to:

```text
Clothing
├── Top
│   ├── Shirt
│   ├── TShirt
│   ├── Hoodie
│   └── Sweater
│
├── Bottom
│   ├── Jeans
│   ├── Cargo
│   ├── Trousers
│   └── Sweatpants
│
└── Shoes
    ├── Sneakers
    ├── Boots
    └── FormalShoes
```

Use:

* Encapsulation
* Inheritance
* Abstraction
* Polymorphism
* Virtual functions
* Dynamic binding
* Constructors
* Destructors
* Friend function
* Static member

These concepts should have legitimate uses in the project.

Do not add meaningless OOP constructs only to claim that they exist.

---

## Required Clothing Attributes

Each clothing item should have:

```text
id
type/category
subtype
color
occasion tags
season tags
```

Optional lightweight attributes:

```text
laundry status
favourite
usage count
```

---

## Controlled Input

Do not allow arbitrary subtype strings from the frontend.

Use predefined values for:

```text
Type
Subtype
Color
Occasion
Season
```

This reduces invalid states and makes the recommendation engine deterministic.

---

## Recommendation Requirements

An outfit must contain:

```text
one top
one bottom
one pair of shoes
```

The recommendation engine must consider:

```text
occasion
season
color
top/bottom compatibility
top/shoe compatibility
bottom/shoe compatibility
```

Implement subtype compatibility using clear rule tables or functions.

Do not use AI/ML.

Return several ranked outfits.

Return an explanation with each recommendation.

---

## Required API

At minimum implement:

```text
GET    /api/clothes
POST   /api/clothes
PUT    /api/clothes/{id}
DELETE /api/clothes/{id}

POST   /api/outfits/generate

GET    /api/health
```

Use JSON request/response bodies.

---

## Required Frontend

Build a simple responsive web interface containing:

```text
Dashboard
Add Clothing
Wardrobe View
Search/Filter
Recommendation Page
```

The frontend communicates exclusively with the C++ backend API.

Do not duplicate recommendation logic in JavaScript.

---

## Persistence

Store wardrobe items in CSV.

The backend must:

```text
load CSV on startup
save changes to CSV
```

The application must retain wardrobe data after restarting.

Keep repository logic separate enough that CSV can later be replaced with SQLite.

---

## Scope Control

Do not add:

```text
authentication
microservices
Docker
cloud infrastructure
ML models
computer vision
external AI APIs
complex databases
payment systems
social features
```

unless specifically requested later.

The goal is a polished, understandable college project rather than a production platform.

---

## Definition of Done

The project is considered complete when:

```text
[ ] Backend compiles using CMake
[ ] C++ REST API runs locally
[ ] Frontend communicates with backend
[ ] Clothing can be added
[ ] Clothing can be edited
[ ] Clothing can be deleted
[ ] Wardrobe can be viewed
[ ] Wardrobe can be filtered
[ ] CSV persistence works
[ ] Outfit generation works
[ ] Occasion rules work
[ ] Season rules work
[ ] Color rules work
[ ] Subtype compatibility works
[ ] Multiple outfits are returned
[ ] Invalid combinations are filtered/ranked appropriately
[ ] OOP concepts are clearly demonstrated
[ ] Project can be explained using its architecture
[ ] Project can be demonstrated entirely on a local machine
```

---

# 32. Project Philosophy

Smart Wardrobe is deliberately **small and understandable**.

The strength of the project should come from:

```text
Good OOP design
+
Clear recommendation rules
+
Working C++ backend
+
Usable frontend
+
Persistent data
```

rather than from the number of technologies used.

The final application should feel like a real small product while remaining sufficiently simple that every member of the team can understand and explain the code.

---

# 33. One-Line Description

> **Smart Wardrobe is a C++-backed web application that organizes a user's wardrobe and generates practical everyday outfits using rule-based compatibility between clothing type, subtype, color, occasion, and season.**
