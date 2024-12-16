import matplotlib.pyplot as plt
import pandas as pd

# Read from CSV file
df_csv = pd.read_csv("results.csv")

# Sort data by Width
df_csv = df_csv.sort_values("Width")

# Create the plot
plt.figure(figsize=(12, 8))

# Scatter plot with a line connecting all points
plt.scatter(
    df_csv["Width"],
    df_csv["Ops"],
    color="blue",
    label="Número de Operações vs Tamanho",
    alpha=0.7,
)
plt.plot(df_csv["Width"], df_csv["Ops"], color="blue", linestyle="--", alpha=0.7)

# Mark the points
for i, row in df_csv.iterrows():
    plt.text(row["Width"], row["Ops"], f'{row["Ops"]}', fontsize=8, ha="right")

# Labels and title
plt.title("Número de operações vs Tamanho da Imagem (ImageAND Básico)", fontsize=14)
plt.xlabel("Tamanho", fontsize=14)
plt.ylabel("Número de Operações", fontsize=14)
plt.grid(alpha=0.3)
plt.legend()
plt.show()
