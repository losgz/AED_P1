import matplotlib.pyplot as plt
import pandas as pd

# Read from the CSV file
df_csv = pd.read_csv("results.csv")

# Separate the data based on cases
df_csv["Case"] = df_csv["Image"].str.split("_").str[0]

# Group by cases for connecting lines
best_case_csv = df_csv[df_csv["Case"] == "best"].sort_values("Width")
worst_case_csv = df_csv[df_csv["Case"] == "worst"].sort_values("Width")
random_case_csv = df_csv[df_csv["Case"] == "random"].sort_values("Width")

# Create the plot
plt.figure(figsize=(12, 8))

# Plot the scatter points for each case
plt.scatter(
    best_case_csv["Width"],
    best_case_csv["Ops"],
    color="green",
    label="Melhor caso",
    alpha=0.7,
)
plt.scatter(
    worst_case_csv["Width"],
    worst_case_csv["Ops"],
    color="red",
    label="Pior Caso",
    alpha=0.7,
)
plt.scatter(
    random_case_csv["Width"],
    random_case_csv["Ops"],
    color="blue",
    label="Caso Médio",
    alpha=0.7,
)

# Connect the points with lines for each case
plt.plot(
    best_case_csv["Width"],
    best_case_csv["Ops"],
    color="green",
    linestyle="--",
    alpha=0.7,
)
plt.plot(
    worst_case_csv["Width"],
    worst_case_csv["Ops"],
    color="red",
    linestyle="--",
    alpha=0.7,
)
plt.plot(
    random_case_csv["Width"],
    random_case_csv["Ops"],
    color="blue",
    linestyle="--",
    alpha=0.7,
)

# Labels and title
plt.title("Número de operações vs Tamanho da Imagem (ImageAND Optimizado)", fontsize=14)
plt.xlabel("Tamanho", fontsize=14)
plt.ylabel("Número de operações", fontsize=14)
plt.grid(alpha=0.3)
plt.legend()
plt.show()
